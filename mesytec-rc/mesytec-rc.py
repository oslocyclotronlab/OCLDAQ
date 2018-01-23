#!/usr/bin/env python
# -*- coding: utf-8 -*-
# vim: set fileencoding=utf-8 :


import gobject, gtk, gtk.glade
import sys, traceback, datetime, math, os, os.path, time
import ConfigParser

allow_mirror = False
MHV4_MODULE_WRITE_SLEEP = 0.5
STM16_PARAMETER_SLEEP = 0.01

appsdir = os.getenv("UIO_APPLICATIONS") or "/Applications"
basedirs = (".", os.path.join(appsdir,"prog","mesytec-rc","data"))
gladename = "mesytec-rc.glade"
gladefile = None
basedir = None
for d in basedirs:
    try:
        gf = os.path.join(d, gladename)
        print "trying '%s'" % (gf,)
        os.stat(gf)
        gladefile = gf
        basedir = d
        break
    except:
        pass
if not gladefile:
    print >>sys.stderr, "No gladefile found. Exit."
    sys.exit(-1)
sys.path.insert(0, basedir)

import mrcc
import log_csv

RED = gtk.gdk.color_parse("#FF0000")
WHITE = gtk.gdk.color_parse("#FFFFFF")
BLACK = gtk.gdk.color_parse("#000000")

###############################################################################

EQUAL, SIMILAR, DIFFERENT = 0, 1, 2
esd_pixbufs = [gtk.gdk.pixbuf_new_from_file(os.path.join(basedir,x))
               for x in ("equal.xpm","similar.xpm","different.xpm")]

def set_yn_changed(widget, changed):
    widget.set_from_pixbuf(esd_pixbufs[changed and DIFFERENT or EQUAL])

def set_yn_similar(widget):
    widget.set_from_pixbuf(esd_pixbufs[SIMILAR])

###############################################################################

def find_widget(xml, name):
    w = xml.get_widget(name)
    if not w:
        print "widget", name, "not found"
    return w

###############################################################################

def find_entry_xalign(xml, name, xalign=1.0):
    w = find_widget(xml, name)
    if w:
        w.set_property('xalign', xalign)
    return w

###############################################################################

def write_config(config, filename):
    """Helper to write the ConfigParser state in a human-friendly way.

    The options will be written ordered by name, and the file will be
    prefixed with a comment stating use, date, and user.

    config -- the ConfigParser object to write
    filename -- path of the file to write to
    """

    f = open(filename, "w")
    f.write("; configuration for mesytec remote control / SiRi slow control\n")
    try:
        import os, pwd
        p = pwd.getpwuid(os.getuid())
        username = " by %s (%s)" % (p[0], p[4].split(",")[0])
    except Exception:
        username =""
    f.write("; saved on "+str(datetime.datetime.now())+username+"\n\n")
    sections = config.sections()
    sections.sort()
    for s in sections:
        f.write("[" + s + "]\n")
        options = config.options(s)
        options.sort()
        for o in options:
            f.write("%s = %s\n" % (o, config.get(s, o)))
        f.write("\n")

###############################################################################

def parent_window(widget):
    """Helper function to find the parent window of a widget.

    returns the parent window (or None?)
    """

    p = widget
    while p and not isinstance(p, gtk.Window):
        p = p.get_parent()
    return p

###############################################################################
###############################################################################

class Channel:
    """Implements some common logic for modules AND channels."""

    ###########################################################################
    def __init__(self, mbd, module=None):
        """Setup a Channel object.
        
        mbd -- tuple (mrcc-connection, bus, device-number)
        module -- parent module for this channel, or None if this is a module
        changed_bit -- bit for indicating the changed=True state to the module
        """
        self.mbd = mbd
        self.module = module
        self.changed_bit = -1
        self.changed = 0

    ###########################################################################
    def GetParameter(self, address, mirror=False, range=None):
        """Read a parameter from the device.

        address -- address in device memory
        mirror -- whether this address is in normal or mirror memory (different command to set it)
        range -- if not None, repeat reading until the value is in the given range
        returns the value of the parameter as read from device memory"""

        mbd = self.mbd
        count = 10
        while count > 0:
            v = mbd[0].GetParameter(mbd[1], mbd[2], address, mirror)
            if not range or (v>=range[0] and v<=range[1]):
                return v
            count -= 1
        return range[1]

    ###########################################################################
    def SetParameter(self, address, value, mirror=False):
        """Write a parameter to the device.

        address -- address in device memory
        value -- value as to be written to the device
        mirror -- whether this address is in normal or mirror memory (different command to set it)"""

        mbd = self.mbd
        mbd[0].SetParameter(mbd[1], mbd[2], address, value, mirror)

    ###########################################################################
    def ReadConfig(self, config):
        """To be implemented by subclasses: read settings

        config -- ConfigParser object containing the settings.
        returns a list of error infos (string message, bool fatal)"""

        return []

    ###########################################################################
    def WriteConfig(self, config):
        """To be implemented by subclasses: write settings

        config -- ConfigParser object containing the settings."""

        return False

    ###########################################################################
    def ParametersForDevice(self, parlist):
        """To be implemented by subclasses: parameters for bulk writing
        
        When apply is pressed, first a list of all parameters to be written is made
        using this function and ParametersForDeviceAfter (in the Module subclass).
        The list is then written to mirror memory, read back, and if the values
        are as written, the mirror page is copied to normal memory.

        returns a list of tuples (write address, read address, value,
              name, (range min, range max), mirror1st) where the range
              tuple may also be None to indicate that no range check
              is to be done."""
        pass

    ###########################################################################
    def UpdateChangeState(self,bit,changed):
        """Sets or unsets changed-bit (0 to 31)."""
        if changed:
            self.changed |= 1<<bit
        else:
            self.changed &= ~(1<<bit)
        if self.module and self.changed_bit>=0:
            self.module.UpdateChangeState(self.changed_bit, self.IsChanged())

    ###########################################################################
    def IsChanged(self, bit=-1):
        """Queries changed-bit (0 to 31)."""
        if bit >= 0:
            return (self.changed & (1<<bit)) !=0
        else:
            return self.changed !=0

    ###########################################################################
    def UpdateChangeMarkers(self):
        pass

###############################################################################
###############################################################################

class Module(Channel):
    """Implements some commonlogic for modules."""

    ###########################################################################
    def __init__(self, mbd):
        Channel.__init__(self, mbd)
        self.channels = []
        
    ###########################################################################
    def Startup(self, config):
        """Takes some steps for startup.

        1. Read device into memory (and GUI display) using 'UpdateMemFromDevice'
        2. Read config from file into GUI input fields using 'ReadConfig'
           Failure -> GUI input fields using 'UpdateGuiFromMem'

        config -- the ConfigParser to read from
        """

        self.UpdateMemFromDevice()

        # try to read config, and update gui from mem if that fails
        conf_errors = []
        if config:
            try:
                conf_errors = self.ReadConfig(config)
            except ConfigParser.Error, e:
                print "Error while reading config: "+e.message
                conf_errors = [(e.message,False)]
        if not config or len(conf_errors):
            self.UpdateGuiFromMem()
        return conf_errors

    ####################
    def EndisableActions(self, enable):
        """Enable or disable submitting changes, to be used during periodical updates."""
        pass

    ###########################################################################
    def MakeLabel(self):
        """Create a label for the notebook tab of this module.

        The label includes a rename button, which is handled by this class.

        returns a box with label and button
        """

        hbox = gtk.HBox()
        self.label = gtk.Label(self.name)
        hbox.pack_start(self.label, False, True)
        button_rename = gtk.Button()
        button_rename.set_relief(gtk.RELIEF_NONE)

        image = gtk.Image()
        image.set_from_file(os.path.join(basedir,"rename.xpm"))
        button_rename.add(image)

        button_rename.connect("clicked", self.on_rename)
        #if log_sqlite.new_system:
        #    button_rename.set_tooltip_text("Rename this tab.")
        hbox.pack_start(button_rename, False, False)
        hbox.show_all()
        return hbox

    ###########################################################################
    def on_rename(self,button,*args):
        """Callback for the rename button in the tab label.

        Displays a dialog for changing the module name.
        """

        dialog = gtk.Dialog("Rename", parent_window(button),
                            gtk.DIALOG_MODAL,
                            (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                             gtk.STOCK_OK, gtk.RESPONSE_OK))
        dialog.set_default_response(gtk.RESPONSE_OK)

        label = gtk.Label("Please enter a new name\nfor module '%s'" % self.name)
        entry = gtk.Entry()
        entry.set_text(self.name)

        dialog.vbox.pack_start(label, False, True, 0)
        dialog.vbox.pack_start(entry, False, True, 0)

        dialog.show_all()

        response = dialog.run()

        if response == gtk.RESPONSE_OK:
            self.name = entry.get_text()
            self.label.set_text(self.name)

        dialog.destroy()
        return True

    ###########################################################################
    def ReadConfig(self, config):
        """Call ReadConfig for all channels.

        returns a list of error messages
        """
        errors = []
        for c in self.channels:
            errors += c.ReadConfig(config)
        return errors

    ###########################################################################
    def WriteConfig(self, config):
        """Call WriteConfig for all channels.

        returns True if all channels did so
        """
        okay = True
        for c in self.channels:
            okay &= c.WriteConfig(config)
        return okay


    ###########################################################################
    def Apply(self):
        """Implements the apply procedure.

        1. Write GUI input fields to device (WriteGuiToDevice)
        2. Update memory from device (UpdateMemFromDevice)
        3. Update change markers (UpdateChangeMarkers)
        """
        self.WriteGuiToDevice()
        self.UpdateMemFromDevice()
        self.UpdateChangeMarkers()
        
    ###########################################################################
    def ParametersForDeviceAfter(self, parlist):
        """Parameters for bulk writing after the channel data, otherwise
        the same as ParametersForDevice."""
        pass

    ###########################################################################
    def WriteGuiToDevice(self):
        """Ask the subclass for parameter values to be written (using
        ParametersForDevice...), and then send them via mirror memory."""
        # build list of values to be written
        parlist = []
        self.ParametersForDevice(parlist)
        for c in self.channels:
            c.ParametersForDevice(parlist)
        self.ParametersForDeviceAfter(parlist)

        if not len(parlist):
            raise "empty parlist"

        # write all these values to mirror memory
        for address_w, address_r, value, name, range, mirror1st in parlist:
            self.SetParameter(address_w, value, allow_mirror and mirror1st)

        # read back and compare
        for address_w, address_r, value, name, range, mirror1st in parlist:
            if allow_mirror and mirror1st:
                v = self.GetParameter(address_r, True)
                if v != value:
                    raise "failed to read back %s=%d from mirror" % (name, value)

        # copy
        mbd = self.mbd
        if allow_mirror:
            mbd[0].CopyMirror(mbd[1], mbd[2])
        
        ## cannot work, e.g. because of the voltage ramping built into the module
        ## read back from real memory
        #for address_w, address_r, value, name, range in parlist:
        #    v = self.GetParameter(address_r, range=range)
        #    if v != value:
        #        raise "failed to read back %s=%d from real mem" % (name, value)
        
    ###########################################################################
    def UpdateChangeMarkers(self):
        for c in self.channels:
            c.UpdateChangeMarkers()

###############################################################################
###############################################################################

class MHV4Channel(Channel):
    """Describes one channel in a MHV4 module."""

    # changed-bits
    C_ON, C_VOLTAGE, C_IW = range(3)
    
    def __init__(self, mhv4, channel):
        Channel.__init__(self, mhv4.mbd, mhv4)
        self.channel = channel

        # fill "mem" variables with dummy values
        self.channel_on = False
        self.voltage = 0.0
        self.currentwarning = 0
        self.current = 0
        self.pm = -1

    ###########################################################################
    def GetFrame(self):
        """Get a frame containing all the widgets for controlling this channel."""
        
        xml = gtk.glade.XML(gladefile,root="mhv4_channel_frame")
        xml.signal_autoconnect(self)

        label = find_widget(xml,"mhv4_channel_label")
        label.set_text_with_mnemonic("Cha_nnel %d:" % self.channel)

        self.toggle_channel_on = find_widget(xml,"mhv4_c_on")
        self.widget_read_pm = find_widget(xml,"mhv4_c_read_pm") # voltage sign
        self.widget_set_v = find_entry_xalign(xml,"mhv4_c_set_v")     # voltage in V
        self.widget_read_v = find_entry_xalign(xml,"mhv4_c_read_v")   # voltage in V
        self.widget_set_iw = find_entry_xalign(xml,"mhv4_c_set_iw")   # in µA
        self.widget_read_iw = find_entry_xalign(xml,"mhv4_c_read_iw") # in µA
        self.widget_read_i = find_entry_xalign(xml,"mhv4_c_read_i")   # in µA
        self.channel_on_changed = find_widget(xml,"mhv4_c_on_changed")
        self.v_changed = find_widget(xml,"mhv4_c_v_changed")
        self.iw_changed = find_widget(xml,"mhv4_c_iw_changed")

        return find_widget(xml,"mhv4_channel_frame")

    ###########################################################################
    def LogItems(self):
        c = self.channel
        return [("channel_%d_on"%c,True),
                ("channel_%d_polarity"%c,True),
                ("channel_%d_voltage"%c,True),
                ("channel_%d_warning"%c,True),
                ("channel_%d_current"%c,True)]

    ###########################################################################
    def LogValues(self):
        c = self.channel
        return [("channel_%d_on"%c,self.channel_on),
                ("channel_%d_polarity"%c,self.pm),
                ("channel_%d_voltage"%c,self.voltage),
                ("channel_%d_warning"%c,self.currentwarning),
                ("channel_%d_current"%c,self.current)]

    ###########################################################################
    def ReadConfig(self, config):
        """Update GUI 'set' entries from values in the ConfigParser object config."""

        errors = []
        sec = self.module.config_section

        conf_pm = config.getint(sec, "channel_%d_polarity" % self.channel)
        if conf_pm != self.pm:
            errors.append(("mod. '%s' ch. %d: polarity is %s in device and %s in config"
                           % (self.module.name, self.channel, (self.pm==1 and '+' or '-'),
                               (conf_pm==1 and '+' or '-')), True))

        on = (config.getint(sec, "channel_%d_on" % self.channel) != 0)
        self.toggle_channel_on.set_active(on)

        v = config.getfloat(sec, "channel_%d_voltage" % self.channel) # voltage in V
        self.widget_set_v.get_adjustment().set_value(v)
        if v < 0 or v > self.module.v_range:
            errors.append(("mod. '%s' ch. %d: %d V out of range [0..%d]"
                           % (self.module.name, self.channel, v, self.module.v_range), True))

        iw = config.getfloat(sec, "channel_%d_warning" % self.channel) # warning level in µA
        self.widget_set_iw.get_adjustment().set_value(iw)
        iw_max = self.widget_set_iw.get_adjustment().get_property('upper')
        if iw < 0 or iw > iw_max:
            errors.append(("mod. '%s' ch. %d: warning at %2.3f out of range [0..%2.3f]"
                           % (self.module.name, self.channel, iw, iw_max), True))

        a = self.widget_set_v.get_adjustment()
        a.set_property("upper", self.module.v_range)
        a.set_value(self.voltage / 10.0)

        self.UpdateChangeMarkers()

        return errors

    ###########################################################################
    def WriteConfig(self, config):
        """Update config from GUI 'set' entries."""

        sec = self.module.config_section

        on = self.toggle_channel_on.get_active()
        config.set(sec, "channel_%d_on" % self.channel, on and "1" or "0")

        config.set(sec, "channel_%d_polarity" % self.channel, "%d" % self.pm)

        v = self.widget_set_v.get_adjustment().get_value()
        config.set(sec, "channel_%d_voltage" % self.channel, "%.1f" % v) # voltage in V

        iw = self.widget_set_iw.get_adjustment().get_value()
        config.set(sec, "channel_%d_warning" % self.channel, "%.2f" % iw) # warning level in µA
        
        return True

    ###########################################################################
    def GetParameter(self, address, mirror=False, range=None):
        v = Channel.GetParameter(self, address, mirror, range)
        time.sleep(STM16_PARAMETER_SLEEP)
        return v

    ###########################################################################
    def SetParameter(self, address, value, mirror=False):
        Channel.SetParameter(self, address, value, mirror)
        time.sleep(STM16_PARAMETER_SLEEP)

    ###########################################################################
    def UpdateMemFromDevice(self,only_measurements=False):

        self.channel_on = (self.GetParameter(36+self.channel) == 1)
        while True:
            v = self.GetParameter(32+self.channel)
            if abs(v) <= self.module.v_range*10+10:
                break
        self.voltage = abs(v) # & 0x7FFF                                # in 0.1V units
        self.widget_read_v.set_text("%3.1f" % (self.voltage/10.0))

        self.pm = self.GetParameter(46+self.channel)
        self.widget_read_pm.set_text((self.pm==1) and '+' or '-')
        
        self.currentwarning = self.GetParameter(40+self.channel, range=(0,2000)) # in 10nA units
        self.widget_read_iw.set_text("%2.2f" % (self.currentwarning/100.0)) # convert 10nA to 1µA units

        self.current = self.GetParameter(50+self.channel)        # in  1nA units
        self.widget_read_i.set_text("%2.3f" % (self.current/1000.0)) # convert from 1nA to 1µA units
        
    ###########################################################################
    def UpdateGuiFromMem(self):
        a = self.widget_set_v.get_adjustment()
        a.set_property("upper", self.module.v_range)
        a.set_value(self.voltage / 10.0)

        self.widget_set_iw.get_adjustment().set_value(self.currentwarning / 100.0)
        self.toggle_channel_on.set_active(self.channel_on)

        self.UpdateChangeMarkers()

    ###########################################################################
    def ParametersForDevice(self, parlist):
        c = self.channel

        # put all parameters to list
        v = int(self.widget_set_v.get_adjustment().get_value() * 10.0)
        parlist.append( [0+c, 32+c, v, "voltage", (0,self.module.v_range), True] )

        on = self.toggle_channel_on.get_active()
        parlist.append( [4+c, 36+c, on and 1 or 0, "channel on", (0,1), True] )

        # does not work with mirror memory !
        w = int(self.widget_set_iw.get_adjustment().get_value() * 100.0)
        parlist.append( [8+c, 40+c, w, "currentwarning", (0,2000), False] )

    ###########################################################################
    def UpdateChangeMarkers(self):
        self.on_spin_voltage_value_changed()
        self.on_spin_currentwarning_value_changed()
        self.on_toggle_channel_on_toggled()

    ###########################################################################
    def on_toggle_channel_on_toggled(self,*otherargs):
        on = self.toggle_channel_on.get_active()
        changed = (self.channel_on != on)
        
        self.toggle_channel_on.set_label(on and "ON" or "OFF")
        set_yn_changed(self.channel_on_changed, changed)
        self.UpdateChangeState(self.C_ON, changed)

    ###########################################################################
    def on_spin_voltage_value_changed(self,*otherargs):
        v = int(self.widget_set_v.get_value()*10) # convert from 1V to 0.1V units
        changed = (self.voltage != v)

        if changed and (abs(self.voltage-v) < 5):
            # for less than 0.5V difference, show it as similar
            set_yn_similar(self.v_changed)
        else:
            set_yn_changed(self.v_changed, changed)
        self.UpdateChangeState(self.C_VOLTAGE, changed)

    ###########################################################################
    def on_spin_currentwarning_value_changed(self,*otherargs):
        iw = int(self.widget_set_iw.get_value()*100) # convert from 1µA to 10nA units
        changed = (self.currentwarning != iw)
        
        set_yn_changed(self.iw_changed, changed)
        self.UpdateChangeState(self.C_IW, changed)

    ###########################################################################
    def GetVoltages(self):
        """Returns pair (gui voltage, mem voltage) in units of 0.1V.
        """
        return (int(self.widget_set_v.get_value()*10), self.voltage)

    ###########################################################################
    def GetSwitchedOn(self):
        """Returns pair (gui on, mem on).
        """
        return (self.toggle_channel_on.get_active(), self.channel_on)

###############################################################################
###############################################################################

class MHV4Ramping:
    """Control class for ramping MHV4 voltage in more than 5s.
    Use one per module."""

    ###########################################################################
    def __init__(self, module):
        self.module = module
        self.timeout_id = -1
        self.debug = True

    ###########################################################################
    def Setup(self):
        xml = gtk.glade.XML(gladefile,root='mhv4_ramping_dialog')
        xml.signal_autoconnect(self)

        self.dialog = find_widget(xml, 'mhv4_ramping_dialog')
        self.dialog.hide()
        self.messages = find_widget(xml, 'mhv4_ramp_messages')
        self.messagebuf = self.messages.get_buffer()
        self.modulename = find_widget(xml, 'mhv4_r_modulename')
        self.apply    = find_widget(xml, 'mhv4_ramp_apply')
        self.spin_rate = find_widget(xml, 'spin_rate')
        self.checkbox_ramp_up = []
        self.checkbox_ramp_dn = []

        for i, c in enumerate(self.module.channels):
            self.checkbox_ramp_up.append( find_widget(xml, 'mhv4_r_up_ch%d' % i) )
            self.checkbox_ramp_dn.append( find_widget(xml, 'mhv4_r_down_ch%d' % i) )

    ###########################################################################
    def CheckAndPrepare(self, prepare=False):
        self.voltages = []
        can_run = False
        for i, c in enumerate(self.module.channels):
            # ask for gui and mem voltage and on-off-state
            vg, vm = c.GetVoltages()
            self.voltages.append( (vg, vm) )
            og, om = c.GetSwitchedOn()
            # we can ramp up if gui is on and if gui voltage
            # is larger than mem voltage
            up_enabled = og and (vg > vm + 10)
            # we can ramp down if mem is on and gui voltage smaller
            # than mem voltage or gui off
            dn_enabled = om and (vm>10) and ((vg + 10 < vm) or not og)
            if False and self.debug:
                print "ramping: channel %d up=%d dn=%d" % (i, up_enabled, dn_enabled), vg, vm, og, om
            if prepare:
                # check and enable checkboxes accordingly
                self.checkbox_ramp_up[i].set_active(up_enabled)
                if not up_enabled:
                    self.checkbox_ramp_up[i].set_sensitive(False)
                self.checkbox_ramp_dn[i].set_active(dn_enabled)
                if not dn_enabled:
                    self.checkbox_ramp_dn[i].set_sensitive(False)
            if up_enabled or dn_enabled:
                can_run = True
        return can_run

    ###########################################################################
    def run(self, parent_w, modulename):
        if self.CheckAndPrepare(True):
            self.dialog.set_title("Ramping - %s" % modulename)
            self.modulename.set_text(modulename)
            self.apply.set_sensitive(True)
            self.messagebuf.set_text('')
            self.dialog.set_transient_for(parent_w)
            self.dialog.show_all()
            gtk.main()

    ###########################################################################
    def on_close_clicked(self, *args):
        if self.timeout_id >= 0:
            gobject.source_remove(self.timeout_id)
            self.timeout_id = -1
        self.dialog.hide()
        gtk.main_quit()

    ###########################################################################
    def on_apply_clicked(self, *args):
        # prepare lists
        self.ramps   = []
        # calculate rate in V/5s
        rate = int(50*self.spin_rate.get_adjustment().get_value())
        self.messagebuf.set_text('Ramping is in 5s intervals.\n')
        for i, c in enumerate(self.module.channels):
            do_up = self.checkbox_ramp_up[i].get_active()
            do_dn = self.checkbox_ramp_dn[i].get_active()
            if not(do_up or do_dn):
                continue

            vend, vstart = self.voltages[i] # gui, mem
            og, om = c.GetSwitchedOn()
            if self.debug:
                print "origin:", vstart, vend, og, om
            if do_up and not om:
                vstart = 0
            elif do_dn and not og:
                vend = 0
            if self.debug:
                print "stepping:", vstart, vend
            f = do_up and 1 or -1
            r = range(vstart, vend, f*rate)
            if self.debug:
                print "steps:", r
            if not len(r) or r[-1] != vend:
                r.append( vend )
            if self.debug:
                print "steps 2:", r
            self.ramps.append( [c, i, 1, r] )
        #print self.ramps
        # call timeout handler once now, maybe install timeout
        self.apply.set_sensitive(False)
        if self.on_ramping_timeout():
            self.timeout_id = gobject.timeout_add(5000, self.on_ramping_timeout)

    ###########################################################################
    def on_ramping_timeout(self, *args):
        need_more = False

        for i, up in enumerate(self.ramps):
            c, chnum, idx, steps = up
            if idx < len(steps):
                need_more = True
                if self.debug:
                    print "c.SetParameter(%d, %d, False)" % (chnum, steps[idx])
                c.SetParameter(chnum, steps[idx], False)
                if idx == len(steps)-1 and steps[idx] == 0:
                    if self.debug:
                        print "switching channel %d off" % (i+1)
                    c.SetParameter(chnum+4, 0, False)
                elif idx == 1 and steps[0] == 0:
                    if self.debug:
                        print "switching channel %d on" % (i+1)
                    c.SetParameter(chnum+4, 1, False)

                end_iter = self.messagebuf.get_end_iter()
                self.messagebuf.insert(self.messagebuf.get_end_iter(),
                                       "Set channel %d to %3.1fV.\n" % (i, steps[idx]/10.0))
                self.ramps[i][2] += 1

        if not need_more:
            self.dialog.hide()
            gtk.main_quit()
            self.module.UpdateMemFromDevice()
            self.module.UpdateChangeMarkers()
        return need_more
        

###############################################################################
###############################################################################

class MHV4(Module):
    """Describes one MHV4 module.

    Uses MHV4Channel to describe the channels.
    """

    device_id = 17
    C_RC, C_MAXVOLTAGE, C_CHANNELS_START = 0, 1, 2 # C_CHANNELS_START must be last

    ###########################################################################
    def __init__(self, mbd):
        Module.__init__(self, mbd)
        self.channels = [MHV4Channel(self,channel) for channel in range(4)]
        for i, c in enumerate(self.channels):
            c.changed_bit = i+self.C_CHANNELS_START
        
        # fill "mem" variables with dummy values
        self.rc_enabled = False
        self.v_range = 0

        # name is only in config file
        self.name = "MHV4 %d %x" % (self.mbd[1], self.mbd[2])
        self.config_section = "MHV4-%d-%X" % (self.mbd[1], self.mbd[2])

        self.ramping = MHV4Ramping(self)

    ###########################################################################
    def PrepareLog(self, log):
        items = [("rc_enabled",True), ("v_range",True)]
        for c in self.channels:
            items.extend( c.LogItems() )
        log.prepare(self.config_section, items)
        
    ###########################################################################
    def Log(self, log):
        items = [("rc_enabled",self.rc_enabled and 1 or 0),
                 ("v_range",self.v_range)]
        for c in self.channels:
            items.extend( c.LogValues() )
        log.log(self.config_section, items)
    
    ###########################################################################
    def ReadConfig(self, config):
        """Update GUI 'set' entries from values in the ConfigParser object config."""

        errors = []
        sec = self.config_section
        if not config.has_section(sec):
            return errors

        rc = config.getint(sec, "remote_control")
        self.toggle_rc_enabled.set_active(rc != 0)

        self.name = config.get(sec, "name")
        self.label.set_text(self.name)

        conf_range = config.getint(sec, "range")
        self.radio_range100V.set_active(conf_range == 100)
        self.radio_range400V.set_active(conf_range == 400)

        # read channels' config
        errors += Module.ReadConfig(self, config)

        self.UpdateChangeMarkers()

        return errors

    ###########################################################################
    def WriteConfig(self, config):
        sec = self.config_section
        if not config.has_section(sec):
            config.add_section(sec)

        rc = self.toggle_rc_enabled.get_active()
        config.set(sec, "remote_control", rc and "1" or "0")

        config.set(sec, "name", self.name)

        config.set(sec, "range", "%d" % self.v_range)

        # write channels' config
        return Module.WriteConfig(self, config)

    ###########################################################################
    def UpdateMemFromDevice(self, only_measurements=False):
        if not only_measurements:
            self.v_range = self.GetParameter(45) and 400 or 100
            self.rc_enabled = (self.GetParameter(44) == 1)

        for c in self.channels:
            c.UpdateMemFromDevice(only_measurements)
        
    ###########################################################################
    def UpdateGuiFromMem(self):
        self.toggle_rc_enabled.set_active(self.rc_enabled)
        if self.v_range == 400:
            self.radio_range400V.set_active(True)
            self.radio_range100V.set_active(False)
        else:
            self.radio_range100V.set_active(True)
            self.radio_range400V.set_active(False)

        for c in self.channels:
            c.UpdateGuiFromMem()

        self.UpdateChangeMarkers()

    ###########################################################################
    def UpdateChangeMarkers(self):
        # update change markers for channels
        Module.UpdateChangeMarkers(self)
        # this call will compare the mem and gui states and set the marker bits
        self.on_toggle_rc_enabled_toggled()
        self.on_range100V_toggled()

        # if ramping is possible, enable the ramping button
        ramping = self.toggle_rc_enabled.get_active() and \
                  self.ramping.CheckAndPrepare()
        self.ramping_button.set_sensitive( ramping )

    ###########################################################################
    def UpdateChangeState(self,bit,changed):
        Module.UpdateChangeState(self, bit, changed)
        self.button_apply.set_sensitive(self.IsChanged())

        # if ramping is possible, enable the ramping button
        ramping = self.toggle_rc_enabled.get_active() and \
                  self.ramping.CheckAndPrepare()
        self.ramping_button.set_sensitive( ramping )

    ###########################################################################
    def WriteGuiToDevice(self):
        # must use special care here (i.e. not IsChanged()) because rc
        # must be enabled before it is used
        gui_rc_enabled = self.toggle_rc_enabled.get_active()
        gui_rc_turned_on = not self.rc_enabled and gui_rc_enabled
        if gui_rc_turned_on:
            self.mbd[0].SetRC(self.mbd[1], self.mbd[2], True)
            time.sleep(MHV4_MODULE_WRITE_SLEEP)

        if (self.changed & (1<<self.C_MAXVOLTAGE)) or gui_rc_turned_on:
            rangeflag = self.radio_range400V.get_active() and 1 or 0
            self.SetParameter(13, rangeflag, False)
            time.sleep(MHV4_MODULE_WRITE_SLEEP)
        
        if self.changed & ~((1<<self.C_CHANNELS_START)-1):
            Module.WriteGuiToDevice(self)

        if self.rc_enabled and not gui_rc_enabled:
            self.mbd[0].SetRC(self.mbd[1], self.mbd[2], False)
            time.sleep(MHV4_MODULE_WRITE_SLEEP)
        
    ###########################################################################
    def GetPage(self):
        """Return a tuple (page,label) with controls for this module."""
        
        xml = gtk.glade.XML(gladefile,root="mhv4_m_box")
        xml.signal_autoconnect(self)
        self.xml = xml

        channel_box = find_widget(xml,"mhv4_channels_box")
        for c in self.channels:
            channel_box.add(c.GetFrame())
        
        self.toggle_rc_enabled = find_widget(xml,"mhv4_m_toggle_rc")
        self.radio_range100V = find_widget(xml,"mhv4_m_radio_100V")
        self.radio_range400V = find_widget(xml,"mhv4_m_radio_400V")
        self.button_apply = find_widget(xml,"mhv4_m_button_apply")
        self.button_refresh = find_widget(xml,"mhv4_m_button_refresh")
        self.rc_changed = find_widget(xml,"mhv4_m_rc_changed")
        self.v_changed  = find_widget(xml,"mhv4_m_v_changed")
        self.ramping_button = find_widget(xml,"mhv4_m_start_ramping")

        entry_bus = find_widget(xml,"mhv4_m_entry_bus")
        entry_bus.set_text("%d" % self.mbd[1])
        entry_dev = find_widget(xml,"mhv4_m_entry_dev")
        entry_dev.set_text("%X" % self.mbd[2])

        self.box = find_widget(xml,"mhv4_m_box")

        self.ramping.Setup()

        return (self.box, self.MakeLabel())

    ####################
    def EndisableActions(self, enable):
        """Enable or disable submitting changes, to be used during periodical updates."""

        self.button_apply.set_sensitive(enable and self.IsChanged())
        self.button_refresh.set_sensitive(enable)
        self.ramping_button.set_sensitive(enable and self.ramping.CheckAndPrepare())

    ###########################################################################
    # now the callbacks, connected via xml.signal_autoconnect(...)
    def on_toggle_rc_enabled_toggled(self,*otherargs):
        on = self.toggle_rc_enabled.get_active()
        changed = (self.rc_enabled != on)
        
        self.toggle_rc_enabled.set_label(on and "ON" or "OFF")
        set_yn_changed(self.rc_changed, changed)
        self.UpdateChangeState(self.C_RC, changed)
        
    ####################
    def on_range100V_toggled(self,*otherargs):
        range100V = self.radio_range100V.get_active()
        changed = (self.v_range != (range100V and 100 or 400))

        set_yn_changed(self.v_changed, changed)
        self.UpdateChangeState(self.C_MAXVOLTAGE, changed)

    ####################
    def on_range400V_toggled(self,*otherargs):
        range400V = self.radio_range400V.get_active()
        changed = (self.v_range != (range400V and 400 or 100))

        set_yn_changed(self.v_changed, changed)
        self.UpdateChangeState(self.C_MAXVOLTAGE, changed)

    ####################
    def on_button_apply_clicked(self,*otherargs):
        self.Apply()

    ####################
    def on_button_device_clicked(self,*otherargs):
        self.UpdateMemFromDevice()
        self.UpdateGuiFromMem()

    ####################
    def on_button_ramping_clicked(self,*otherargs):
        self.ramping.run(parent_window(self.box), self.name)

###############################################################################
###############################################################################

class STM16Pair(Channel):
    """Describes a pair of channels in a STM16 module."""
    
    # changed-bits
    C_GAIN, C_THRESHOLD_A, C_THRESHOLD_B = range(3)
    
    def __init__(self, stm16, channel):
        Channel.__init__(self, stm16.mbd, stm16)
        self.channel = channel

        # fill "mem" variables with dummy values
        self.gain = False
        self.threshold_a = 0
        self.threshold_b = 0
        self.synchro = []

    ###########################################################################
    def GetFrame(self):
        """Get a frame containing all the widgets for controlling this channel."""
        
        xml = gtk.glade.XML(gladefile,root="stm16_channel_frame")
        xml.signal_autoconnect(self)

        label = find_widget(xml,"stm16_channel_label")
        label.set_text_with_mnemonic("Cha_nnels %d & %d:" % (self.channel, self.channel+1))

        self.widget_read_gain = find_entry_xalign(xml,"stm16_c_read_gain")
        self.widget_info_gain = find_entry_xalign(xml,"stm16_c_info_gain")
        self.widget_set_gain  = find_entry_xalign(xml,"stm16_c_set_gain")
        self.widget_read_threshold_a = find_entry_xalign(xml,"stm16_c_read_threshold_a")
        self.widget_info_threshold_a = find_entry_xalign(xml,"stm16_c_info_threshold_a")
        self.widget_set_threshold_a = find_entry_xalign(xml,"stm16_c_set_threshold_a")
        self.widget_read_threshold_b = find_entry_xalign(xml,"stm16_c_read_threshold_b")
        self.widget_info_threshold_b = find_entry_xalign(xml,"stm16_c_info_threshold_b")
        self.widget_set_threshold_b = find_entry_xalign(xml,"stm16_c_set_threshold_b")
        self.image_gain_changed  = find_widget(xml,"stm16_c_gain_changed")
        self.image_ta_changed  = find_widget(xml,"stm16_c_threshold_a_changed")
        self.image_tb_changed  = find_widget(xml,"stm16_c_threshold_b_changed")

        find_widget(xml,"stm16_c_gain_label").set_text("Gain %d & %d" % (self.channel, self.channel+1))
        find_widget(xml,"stm16_c_threshold_label_a").set_text("Threshold %d" % self.channel)
        find_widget(xml,"stm16_c_threshold_label_b").set_text("Threshold %d" % (self.channel+1))

        return find_widget(xml,"stm16_channel_frame")

    ###########################################################################
    def ReadConfig(self, config):
        """Update GUI 'set' entries from values in the ConfigParser object config."""

        errors = []
        sec = self.module.config_section

        g = config.getint(sec, "gain_%02d_%02d" % (self.channel, self.channel+1))
        self.widget_set_gain.get_adjustment().set_value(g)
        g_max = self.widget_set_gain.get_adjustment().get_property('upper')
        if g < 0 or g > g_max:
            errors.append(("mod. '%s' ch. %d/%d: gain %d out of range [0..%d]"
                           % (self.module.name, self.channel, self.channel+1, g, g_max), True))

        ta = config.getint(sec, "threshold_%02d" % self.channel)
        self.widget_set_threshold_a.get_adjustment().set_value(ta)
        ta_max = self.widget_set_threshold_a.get_adjustment().get_property('upper')
        if ta < 0 or ta > ta_max:
            errors.append(("mod. '%s' ch. %d: threshold %d out of range [0..%d]"
                           % (self.module.name, self.channel, ta, ta_max), True))

        tb = config.getint(sec, "threshold_%02d" % (self.channel+1))
        self.widget_set_threshold_b.get_adjustment().set_value(tb)
        tb_max = self.widget_set_threshold_b.get_adjustment().get_property('upper')
        if tb < 0 or tb > tb_max:
            errors.append(("mod. '%s' ch. %d: threshold %d out of range [0..%d]"
                           % (self.module.name, self.channel, tb, tb_max), True))

        self.UpdateChangeMarkers()

        return errors

    ###########################################################################
    def WriteConfig(self, config):
        sec = self.module.config_section
        c = self.channel

        g = self.widget_set_gain.get_adjustment().get_value()
        config.set(sec, "gain_%02d_%02d" % (c, c+1), "%d" % g)

        ta = self.widget_set_threshold_a.get_adjustment().get_value()
        config.set(sec, "threshold_%02d" % c, "%d" % ta)

        tb = self.widget_set_threshold_b.get_adjustment().get_value()
        config.set(sec, "threshold_%02d" % (c+1), "%d" % tb)

        return True

    ###########################################################################
    def LogItems(self):
        c = self.channel
        return [("gain_%02d_%02d" % (c, c+1),True),
                ("threshold_%02d" % c,True),
                ("threshold_%02d" % (c+1),True)]

    ###########################################################################
    def LogValues(self):
        c = self.channel
        return [("gain_%02d_%02d" % (c, c+1),self.gain),
                ("threshold_%02d" % c,self.threshold_a),
                ("threshold_%02d" % (c+1),self.threshold_b)]

    ###########################################################################
    def UpdateMemFromDevice(self,only_measurements=False):

        self.gain = self.GetParameter(2*self.channel)
        self.threshold_a = self.GetParameter(2*self.channel+1)
        self.threshold_b = self.GetParameter(2*self.channel+3)

        self.widget_read_gain.set_text("%d" % self.gain)
        self.widget_read_threshold_a.set_text("%d" % self.threshold_a)
        self.widget_read_threshold_b.set_text("%d" % self.threshold_b)
        
    ###########################################################################
    def UpdateGuiFromMem(self):
        self.widget_set_gain.get_adjustment().set_value(self.gain)
        self.widget_set_threshold_a.get_adjustment().set_value(self.threshold_a)
        self.widget_set_threshold_b.get_adjustment().set_value(self.threshold_b)

        self.UpdateChangeMarkers()

    ###########################################################################
    def ParametersForDevice(self, parlist):
        c = self.channel

        # put all parameters to list
        g = int(self.widget_set_gain.get_adjustment().get_value())
        parlist.append( [2*c, 2*c, g, "gain", (0,15), True] )

        ta = int(self.widget_set_threshold_a.get_adjustment().get_value())
        parlist.append( [2*c+1, 2*c+1, ta, "threshold a", (0,255), True] )

        tb = int(self.widget_set_threshold_b.get_adjustment().get_value())
        parlist.append( [2*c+3, 2*c+3, tb, "threshold a", (0,255), True] )

    ###########################################################################
    def SetIsSynchronized(self, synchro):
        self.widget_set_gain.set_sensitive(not synchro)
        self.widget_set_threshold_a.set_sensitive(not synchro)
        self.widget_set_threshold_b.set_sensitive(not synchro)

    ###########################################################################
    def UpdateChangeMarkers(self):
        self.on_gain_value_changed()
        self.on_threshold_a_value_changed()
        self.on_threshold_b_value_changed()

    ###########################################################################
    def on_gain_value_changed(self,*args):
        g = self.widget_set_gain.get_adjustment().get_value()
        changed = (self.gain != g)

        for pair in self.synchro:
            pair.widget_set_gain.get_adjustment().set_value(g)
        
        set_yn_changed(self.image_gain_changed, changed)
        self.widget_info_gain.set_text("%2.2f" % math.pow(1.22, g))
        self.UpdateChangeState(self.C_GAIN, changed)

    ###########################################################################
    def on_threshold_a_value_changed(self,*args):
        ta = self.widget_set_threshold_a.get_adjustment().get_value()
        changed = (self.threshold_a != ta)
        
        for pair in self.synchro:
            pair.widget_set_threshold_a.get_adjustment().set_value(ta)
        
        set_yn_changed(self.image_ta_changed, changed)
        self.widget_info_threshold_a.set_text("%d" % int(ta/255.0*4000)) # assuming max=4000mV
        self.UpdateChangeState(self.C_THRESHOLD_A, changed)

    ###########################################################################
    def on_threshold_b_value_changed(self,*args):
        tb = self.widget_set_threshold_b.get_adjustment().get_value()
        changed = (self.threshold_b != tb)
        
        for pair in self.synchro:
            pair.widget_set_threshold_b.get_adjustment().set_value(tb)
        
        set_yn_changed(self.image_tb_changed, changed)
        self.widget_info_threshold_b.set_text("%d" % (tb/255.0*4000)) # assuming max=4000mV
        self.UpdateChangeState(self.C_THRESHOLD_B, changed)

###############################################################################
###############################################################################

class STM16(Module):
    """Describes one STM16 module.

    Uses STM16Pair to describe the channels.
    """

    device_id = 19
    C_RC, C_CHANNELS_START = 0, 1

    ###########################################################################
    def __init__(self, mbd):
        Module.__init__(self, mbd)
        self.channels = [STM16Pair(self,channel) for channel in range(0,16,2)]
        for i, c in enumerate(self.channels):
            c.changed_bit = i+self.C_CHANNELS_START
        
        # fill "mem" variables with dummy values
        self.rc_enabled = False

        # name is only in config file
        self.name = "STM16 %d %x" % (self.mbd[1], self.mbd[2])
        self.config_section = "STM16-%d-%X" % (self.mbd[1], self.mbd[2])

    ###########################################################################
    def ReadConfig(self, config):
        """Update GUI 'set' entries from values in the ConfigParser object config."""

        sec = self.config_section
        if not config.has_section(sec):
            return []

        rc = config.getint(sec, "remote_control")
        self.toggle_rc_enabled.set_active(rc != 0)

        self.name = config.get(sec, "name")
        self.label.set_text(self.name)

        self.UpdateChangeMarkers()

        # read channels' config
        return Module.ReadConfig(self, config)

    ###########################################################################
    def PrepareLog(self, log):
        items = [("rc_enabled",True)]
        for c in self.channels:
            items.extend( c.LogItems() )
        log.prepare(self.config_section, items)
        
    ###########################################################################
    def Log(self, log):
        items = [("rc_enabled",self.rc_enabled and 1 or 0)]
        for c in self.channels:
            items.extend( c.LogValues() )
        log.log(self.config_section, items)
    
    ###########################################################################
    def WriteConfig(self, config):
        sec = self.config_section
        if not config.has_section(sec):
            config.add_section(sec)

        rc = self.toggle_rc_enabled.get_active()
        config.set(sec, "remote_control", rc and "1" or "0")

        config.set(sec, "name", self.name)

        # write channels' config
        return Module.WriteConfig(self, config)

    ###########################################################################
    def UpdateMemFromDevice(self, only_measurements=False):
        if not only_measurements:
            scan = self.mbd[0].ScanBus(self.mbd[1])
            self.rc_enabled, = [s[2] for s in scan if s[0]==self.mbd[2]]

        for c in self.channels:
            c.UpdateMemFromDevice(only_measurements)
        
    ###########################################################################
    def UpdateGuiFromMem(self):
        
        self.toggle_rc_enabled.set_active(self.rc_enabled)

        for c in self.channels:
            c.UpdateGuiFromMem()

        self.UpdateChangeMarkers()

    ###########################################################################
    def UpdateChangeMarkers(self):
        self.on_toggle_rc_enabled_toggled()
        Module.UpdateChangeMarkers(self)

    ###########################################################################
    def UpdateChangeState(self,bit,changed):
        Channel.UpdateChangeState(self, bit, changed)
        self.button_apply.set_sensitive(self.IsChanged())

    ###########################################################################
    def WriteGuiToDevice(self):
        # must use special care here (i.e. not IsChanged()) because rc
        # must be enabled before it is used
        gui_rc_enabled = self.toggle_rc_enabled.get_active()
        if not self.rc_enabled and gui_rc_enabled:
            self.mbd[0].SetRC(self.mbd[1], self.mbd[2], True)
        
        if self.changed & ~(1<<self.C_RC):
            Module.WriteGuiToDevice(self)

        if self.rc_enabled and not gui_rc_enabled:
            self.mbd[0].SetRC(self.mbd[1], self.mbd[2], False)
        
    ###########################################################################
    def GetPage(self):
        """Return a tuple (page,label) with controls for this module."""
        
        xml = gtk.glade.XML(gladefile,root="stm16_m_box")
        xml.signal_autoconnect(self)

        channel_box = find_widget(xml,"stm16_channels_box")
        # put channels in reverse order to make Alt-N shortcut go to the
        # channel-pairs in normal order
        for i in range(len(self.channels)-1,-1,-1):
            c = self.channels[i]
            left = i / 4; top = i % 4
            channel_box.attach(c.GetFrame(), left, left+1, top, top+1,
                               gtk.FILL, gtk.FILL)
        
        self.toggle_rc_enabled = find_widget(xml,"stm16_m_toggle_rc")
        self.image_rc_changed = find_widget(xml,"stm16_m_rc_changed")
        self.button_apply = find_widget(xml,"stm16_m_button_apply")
        self.button_refresh = find_widget(xml,"stm16_m_button_refresh")
        self.sync_off = find_widget(xml,"stm16_m_sync_off")
        self.sync_07_815 = find_widget(xml,"stm16_m_sync_07_815")
        self.sync_015 = find_widget(xml,"stm16_m_sync_015")

        self.sync_off.set_active(True)

        entry_bus = find_widget(xml,"stm16_m_entry_bus")
        entry_bus.set_text("%d" % self.mbd[1])
        entry_dev = find_widget(xml,"stm16_m_entry_dev")
        entry_dev.set_text("%X" % self.mbd[2])

        self.box = find_widget(xml,"stm16_m_box")

        return (self.box, self.MakeLabel())

    ####################
    def EndisableActions(self, enable):
        """Enable or disable submitting changes, to be used during periodical updates."""

        self.button_apply.set_sensitive(enable and self.IsChanged())
        self.button_refresh.set_sensitive(enable)

    ###########################################################################
    # now the callbacks, connected via xml.signal_autoconnect(...)
    def on_toggle_rc_enabled_toggled(self,*otherargs):
        on = self.toggle_rc_enabled.get_active()
        changed = (self.rc_enabled != on)
        
        self.toggle_rc_enabled.set_label(on and "ON" or "OFF")
        set_yn_changed(self.image_rc_changed, changed)
        self.UpdateChangeState(0, changed)
        
    ####################
    def on_button_apply_clicked(self,*otherargs):
        self.Apply()

    ####################
    def on_button_device_clicked(self,*otherargs):
        self.UpdateMemFromDevice()
        self.UpdateGuiFromMem()

    ####################
    def on_stm16_m_sync_toggled(self,*otherargs):
        if self.sync_off.get_active():
            self.channels[0].synchro = []
            self.channels[4].synchro = []
            for i in range(1,8):
                self.channels[i].SetIsSynchronized(False)
        elif self.sync_07_815.get_active():
            self.channels[0].synchro = self.channels[1:4]
            self.channels[4].synchro = self.channels[5:8]
            self.channels[4].SetIsSynchronized(False)
            for i in (1,2,3,5,6,7):
                self.channels[i].SetIsSynchronized(True)
        elif self.sync_015.get_active():
            self.channels[0].synchro = self.channels[1:8]
            self.channels[4].SetIsSynchronized(True)
            for i in range(1,8):
                self.channels[i].SetIsSynchronized(True)

###############################################################################
###############################################################################

class MainWindow:

    ###########################################################################
    def __init__(self, mrcc_dev, logfile, conffile):
        print "MRCC device is '%s'." % mrcc_dev
        
        self.mrcc = mrcc.connect(mrcc_dev)
        self.log = log_csv.Log_csv()
        if logfile:
            print "Logging to '%s'." % logfile
            self.log.open(logfile)
        else:
            print "No logging."
        if conffile:
            self.filename = os.path.abspath(conffile)
        else:
            self.filename = None

        self.xml = gtk.glade.XML(gladefile,root="main_window")
        self.xml.signal_autoconnect(self)

        self.notebook = find_widget(self.xml,"main_notebook")
        self.timeout_period = find_entry_xalign(self.xml,"main_timeout")
        self.main_statusbar = find_widget(self.xml,"main_statusbar")
        self.status_ctx = self.main_statusbar.get_context_id("messages")
        self.main_window = find_widget(self.xml,"main_window")
        self.main_window.set_title("SiRi Slow Control - '%s'" % mrcc_dev)

        print "Scanning bus..."
        self.ScanBus()
        
        if self.filename:
            print "Reading config file '%s' ..." % self.filename
            config = ConfigParser.ConfigParser()
            try:
                config.read([self.filename])
            except ConfigParser.Error, e:
                m = gtk.MessageDialog(None, gtk.DIALOG_MODAL, gtk.MESSAGE_WARNING,
                                      gtk.BUTTONS_OK)
                m.set_title("SiRi Slow Control -- Config Read Error")
                m.set_markup("<b>The configuration file is unreadable and will be ignored.</b>\n"
                             +e.message)
                m.run()
                m.destroy()
                config = None
        else:
            config = None

        print "Initializing module settings..."
        errors = []
        for m in self.modules:
            errors += m.Startup(config)
        self.HandleConfigErrors(errors)

        if logfile:
            print "Preparing logging..."
            self.log.start_prepare()
            for m in self.modules:
                m.PrepareLog(self.log)
            self.log.stop_prepare()

        self.main_window.show()

        # install default timeout as set in the glade file
        self.timeout_id = -1
        self.on_timeout_changed()

    ###########################################################################
    def push_status(self, message):
        self.main_statusbar.push(self.status_ctx, message)
        while gtk.events_pending():
            gtk.main_iteration()

    ###########################################################################
    def pop_status(self):
        self.main_statusbar.pop(self.status_ctx)
        while gtk.events_pending():
            gtk.main_iteration()

    ###########################################################################
    def on_timeout_changed(self, *args):
        # if old timeout exists, stop it
        if self.timeout_id >= 0:
            gobject.source_remove(self.timeout_id)
        # get value from spinbutton and convert s -> ms
        tout = int(self.timeout_period.get_adjustment().get_value()) * 1000
        # add new timeout source
        self.timeout_id = gobject.timeout_add(tout, self.idle_timeout_update)

    ###########################################################################
    def idle_timeout_update(self, *args):
        # block action input
        for m in self.modules:
            m.EndisableActions(False)

        # read values from devices
        self.push_status("Updating. Please wait.")

        try:
            for m in self.modules:
                m.UpdateMemFromDevice(True)
                m.UpdateChangeMarkers()
        except Exception, e:
            print "Time: "+str(datetime.datetime.now())+"\nException while updating: "+str(e)+"\n"
        self.pop_status()

        # log files to database
        ## this might cause problems if quit is pressed
        self.push_status("Logging. Please wait.")
        self.log.start_log()
        for m in self.modules:
            m.Log(self.log)
        self.log.stop_log()
        self.pop_status()

        # done, unblock input
        for m in self.modules:
            m.EndisableActions(True)
        return True

    ###########################################################################
    def ScanBus(self):
        self.modules = []
        for bus in (0,1):
            modules = self.mrcc.ScanBus(bus)
            for address, module_type, rc_state in modules:
                m = None
                if module_type == MHV4.device_id:
                    m = MHV4((self.mrcc, bus, address))
                elif module_type == STM16.device_id:
                    m = STM16((self.mrcc, bus, address))
                else:
                    print "module type %d not implemented" % module_type
                if m:
                    page, label = m.GetPage()
                    #self.notebook.insert_page(page, label, 0)
                    self.notebook.append_page(page, label)
                    self.modules.append(m)
                    m.UpdateChangeMarkers()

    ###########################################################################
    def WriteConfig(self):
        config = ConfigParser.ConfigParser()

        okay = True
        for m in self.modules:
            okay &= m.WriteConfig(config)

        if okay:
            write_config(config, self.filename)
        del config

    ###########################################################################
    def ReadConfig(self):
        if not self.filename:
            return False

        config = ConfigParser.ConfigParser()
        config.read(self.filename)

        errors = []
        for m in self.modules:
            errors += m.ReadConfig(config)

        del config
        self.HandleConfigErrors(errors)

    ###########################################################################
    def HandleConfigErrors(self, errors):
        if not errors:
            return
        
        xml = gtk.glade.XML(gladefile,root="main_config_errors_dialog")
        
        messages = find_widget(xml, "main_cerr_messages")
        messagebuf = messages.get_buffer()
        messagebuf.set_text('')
        
        cont_button = find_widget(xml, "main_cerr_button_mem")
        for m, f in errors:
            if f:
                cont_button.set_sensitive(False)
            messagebuf.insert(messagebuf.get_end_iter(), m+"\n")
                
        dialog = find_widget(xml, "main_config_errors_dialog")
        dialog.set_transient_for(self.main_window)
        r = dialog.run()

        dialog.destroy()
        while gtk.events_pending():
            gtk.main_iteration()
            
        if r == 1 or r == -4:
            # 'Quit' button
            if gtk.main_level():
                self.main_quit()
            else:
                sys.exit(-1)
        else:
            # 'Refresh' button, load from memory
            self.push_status("Loading settings from devices...")
            for m in self.modules:
                m.UpdateMemFromDevice()
                m.UpdateGuiFromMem()
            self.pop_status()

    # gtk callbacks
    ###########################################################################
    def main_quit(self,*otherargs):
        self.log.close()
        self.on_save_as_activate(self.main_window)
        gtk.main_quit()
        return True

    ###########################################################################
    def on_open_activate(self,w,*args):
        dialog = gtk.FileChooserDialog("Open...", parent_window(w),
                                       gtk.FILE_CHOOSER_ACTION_OPEN,
                                       (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                        gtk.STOCK_OPEN, gtk.RESPONSE_OK))
        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_local_only(True)
        if self.filename:
            dialog.set_filename(self.filename)

        r = dialog.run()
        dialog.destroy()

        if r == gtk.RESPONSE_OK:
            self.filename = dialog.get_filename()
            self.ReadConfig()

        return True

    ###########################################################################
    def on_save_activate(self,w,*args):
        if not self.filename:
            return self.on_save_as_activate(w)
        else:
            self.WriteConfig()

    ###########################################################################
    def on_save_as_activate(self,w,*args):
        dialog = gtk.FileChooserDialog("Save as...", parent_window(w),
                                       gtk.FILE_CHOOSER_ACTION_SAVE,
                                       (gtk.STOCK_CANCEL, gtk.RESPONSE_CANCEL,
                                        gtk.STOCK_OPEN, gtk.RESPONSE_OK))
        dialog.set_default_response(gtk.RESPONSE_OK)
        dialog.set_local_only(True)
        if self.filename:
            dialog.set_filename(self.filename)

        r = dialog.run()

        if r == gtk.RESPONSE_OK:
            self.filename = dialog.get_filename()
            print "filename='%s'" % self.filename
            if self.filename:
                self.WriteConfig()

        dialog.destroy()

        return True

    ###########################################################################
    def on_about_activate(self,w,*args):
        xml = gtk.glade.XML(gladefile,root="aboutdialog")
        a = find_widget(xml,"aboutdialog")
        a.set_transient_for(self.main_window)
        a.run()
        a.destroy()

        return True

###############################################################################
def main():
    mrcc_dev = "/dev/ttyUSB0"
    logfile  = "log.csv"
    conffile = None

    argc = len(sys.argv)
    i = 1
    while i < argc:
        if sys.argv[i] == '-dev' and i+1 < argc:
            mrcc_dev = sys.argv[i+1]
            i += 1
        elif sys.argv[i] == '-log' and i+1 < argc:
            logfile = sys.argv[i+1]
            if logfile == "-" or logfile == "None" or logfile == "none":
                logfile = None
            i += 1
        elif not conffile:
            conffile = sys.argv[i]
        else:
            print "ERROR: two conffiles given or command line not understood."
            print "       Use like:"
            print "      ", sys.argv[0], "[-dev mrcc_dev] [-log logfile] [conffile]"
            sys.exit(-1)
        i += 1

    m = MainWindow(mrcc_dev, logfile, conffile)
    gtk.main()

###############################################################################
if __name__ == "__main__":
    main()
