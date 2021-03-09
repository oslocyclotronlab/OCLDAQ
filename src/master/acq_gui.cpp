
#include "acq_gui.h"

#include "acq_log.h"
#include "m_control.h"
#include "m_engine.h"
#include "m_sort.h"
#include "run_command.h"

#include <Xm/CascadeB.h>
#include <Xm/CascadeBG.h>
#include <Xm/DrawnB.h>
#include <Xm/FileSB.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/MainW.h>
#include <Xm/MessageB.h>
#include <Xm/Protocols.h>
#include <Xm/PanedW.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/SeparatoG.h>
#include <Xm/Text.h>
#include <Xm/ToggleB.h>
#include <Xm/ToggleBG.h>
#include <Xm/SelectioB.h>

#include <iostream>
#include <fstream>
#include <sstream>

#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

//#define NDEBUG
#include "debug.h"

XtAppContext app_context;

static Widget toplevel;
static Widget menu_file_engine, menu_file_sort;
static Widget menu_run_start, menu_run_stop, menu_run_status, menu_run_GUI;
static Widget menu_output_device, menu_output_change;
static Widget menu_spectra_clear, menu_spectra_dump;
static Widget menu_gain_init, menu_gain_file, menu_gain_show;
static Widget menu_telewin_init, menu_telewin_file, menu_telewin_show;
static Widget menu_sort_load, menu_sort_show;
static Widget button_start, button_stop, button_status, button_change_file;
static Widget text_output_buffers, text_output_file, text_output_bufrate;
static Widget text_sorting_buffers, text_sorting_errors, text_sorting_avelen;
static Widget text_infos, text_errors;

static std::string last_gain_filename    = /*"../../data/"*/ "gainshift.tmp";
static std::string last_telewin_filename = /*"../../data/"*/ "telewin.tmp";
static std::string last_sort_filename    = "user_routine_mama.cpp";

extern command_list* commands;

/**
 * Utility function to avoid warnings from the c++ compiler for all
 * motif functions expecting 'char*' instead of 'const char*'.
 */
inline char* ccc(const char* c)
{
    return const_cast<char*>(c);
}

// ########################################################################

static bool engine_directory_dialog_showing = false;

// ########################################################################

static void engine_cancel_directory(Widget dialog, XtPointer, XtPointer)
{
    XtDestroyWidget(dialog);
    engine_directory_dialog_showing = false;
}

// ########################################################################

static void engine_change_directory(Widget dialog, XtPointer, XtPointer)
{
    char cwd[1024];
    if( !getcwd(cwd, sizeof(cwd)) ) {
        log_message(LOG_ERR, "acq_master: Cannot get working directory!\n");
        return;
    }

    log_message(LOG_INFO, "Asking engine to change to '%s'\n", cwd);
    m_engine_output_dir(cwd);
    engine_cancel_directory(dialog, 0, 0);
}

// ########################################################################

static void show_engine_dir_dialog(const char* cwd)
{
    if( engine_directory_dialog_showing )
        return;
    engine_directory_dialog_showing = true;
    std::ostringstream message;
    message << "Make engine change directory to '" << cwd << "'?";

    XmString xm_message = XmStringCreateLocalized(ccc(message.str().c_str()));
    Widget dialog = XmCreateWarningDialog(toplevel, ccc("Engine Directory"), NULL, 0);
    XtVaSetValues(dialog,
                  XmNmessageString, xm_message,
                  XmNautoUnmanage,         False,
                  XmNdialogStyle,          XmDIALOG_FULL_APPLICATION_MODAL,
                  NULL);
    XmStringFree(xm_message);
    XtUnmanageChild (XtNameToWidget (dialog, "Help"));

    XtAddCallback (dialog, XmNokCallback, engine_change_directory, NULL);
    XtAddCallback (dialog, XmNcancelCallback, engine_cancel_directory, NULL);

    XtManageChild (dialog);
    //XtPopup (XtParent (dialog), XtGrabNone);
}

// ########################################################################

static void engine_check_directory()
{
    if( !m_engine_is_connected() ) {
        log_message(LOG_ERR, "acq_master: Engine not connected!\n");
        return;
    }

    char cwd[1024];
    if( !getcwd(cwd, sizeof(cwd)) ) {
        log_message(LOG_ERR, "acq_master: Cannot get working directory!\n");
        return;
    }
    const char* edir_c = m_engine_get_output_dir();
    if( !edir_c )
        return;
    const std::string edir(edir_c), mdir(cwd);
    if( edir == mdir )
        return;

    if( m_engine_is_started() ) {
        log_message(LOG_INFO, "Engine started, but in different directory!\n");
        return;
    }
    show_engine_dir_dialog(cwd);
}

// ########################################################################

static bool sort_directory_dialog_showing = false;

// ########################################################################

static void sort_cancel_directory(Widget dialog, XtPointer, XtPointer)
{
    XtDestroyWidget(dialog);
    sort_directory_dialog_showing = false;
}

// ########################################################################

static void sort_change_directory(Widget dialog, XtPointer, XtPointer)
{
    char cwd[1024];
    if( !getcwd(cwd, sizeof(cwd)) ) {
        log_message(LOG_ERR, "acq_master: Cannot get working directory!\n");
        return;
    }

    log_message(LOG_INFO, "Asking sort to change to '%s'\n", cwd);
    m_sort_change_cwd(cwd);
    sort_cancel_directory(dialog, 0, 0);
}

// ########################################################################

static void show_sort_dir_dialog(const char* cwd)
{
    if( sort_directory_dialog_showing )
        return;
    sort_directory_dialog_showing = true;
    std::ostringstream message;
    message << "Make sort change directory to '" << cwd << "'?";

    XmString xm_message = XmStringCreateLocalized(ccc(message.str().c_str()));
    Widget dialog = XmCreateWarningDialog(toplevel, ccc("Sort Directory"), NULL, 0);
    XtVaSetValues(dialog,
                  XmNmessageString, xm_message,
                  XmNautoUnmanage,         False,
                  XmNdialogStyle,          XmDIALOG_FULL_APPLICATION_MODAL,
                  NULL);
    XmStringFree(xm_message);
    XtUnmanageChild (XtNameToWidget (dialog, "Help"));

    XtAddCallback (dialog, XmNokCallback, sort_change_directory, NULL);
    XtAddCallback (dialog, XmNcancelCallback, sort_cancel_directory, NULL);

    XtManageChild (dialog);
    //XtPopup (XtParent (dialog), XtGrabNone);
}

// ########################################################################

static void sort_check_directory()
{
    if( !m_sort_is_connected() ) {
        log_message(LOG_ERR, "acq_master: Sort not connected!\n");
        return;
    }

    char cwd[1024];
    if( !getcwd(cwd, sizeof(cwd)) ) {
        log_message(LOG_ERR, "acq_master: Cannot get working directory!\n");
        return;
    }
    const char* sdir_c = m_sort_get_cwd();
    if( !sdir_c )
        return;
    const std::string sdir(sdir_c), mdir(cwd);
    if( sdir == mdir )
        return;

    show_sort_dir_dialog(cwd);
}

// ########################################################################

/** Update status for all menu options and pushbuttons.
 */
void gui_update_state()
{
    bool engine_connected = m_engine_is_connected();
    XtSetSensitive( button_status,       engine_connected );
    XtSetSensitive( menu_run_status,     engine_connected );
    XtSetSensitive( menu_file_engine,   !engine_connected );

    bool engine_started   = m_engine_is_started();
    XtSetSensitive( button_start,        engine_connected && !engine_started );
    XtSetSensitive( menu_run_start,      engine_connected && !engine_started );
    XtSetSensitive( menu_run_GUI,	     engine_connected );
    XtSetSensitive( button_stop,         engine_started );
    XtSetSensitive( menu_run_stop,       engine_started );
    XtSetSensitive( menu_output_device,  engine_connected /*&& !engine_started*/ );

    const char* output_filename = m_engine_get_output();
    XtSetSensitive( button_change_file, engine_connected && output_filename!=0 );
    XtSetSensitive( menu_output_change, engine_connected && output_filename!=0 );

    if( engine_connected ) {
        if( !output_filename )
            output_filename = "<none>";
        XmTextSetString(text_output_file, ccc(output_filename));

        if( engine_started ) {
            char buf[12];
            snprintf(buf, sizeof(buf), "%8d", m_engine_get_buffercount());
            XmTextSetString(text_output_buffers, buf);
            snprintf(buf, sizeof(buf), "%8.4f", m_engine_get_buffer_rate());
            XmTextSetString(text_output_bufrate, buf);
        } else {
            XmTextSetString(text_output_buffers, ccc("stopped"));
            XmTextSetString(text_output_bufrate, ccc(""));
        }
    } else {
        XmTextSetString(text_output_file, ccc("<not connected>"));
        XmTextSetString(text_output_buffers, ccc("<n.c.>"));
        XmTextSetString(text_output_bufrate, ccc("?"));
    }

    bool sort_connected   = m_sort_is_connected();
    XtSetSensitive( menu_file_sort,     !sort_connected );
    XtSetSensitive( menu_spectra_clear,  sort_connected );
    XtSetSensitive( menu_spectra_dump,   sort_connected );
    XtSetSensitive( menu_gain_init,      sort_connected );
    XtSetSensitive( menu_gain_file,      sort_connected );
    XtSetSensitive( menu_gain_show,      sort_connected );
    XtSetSensitive( menu_telewin_init,   sort_connected );
    XtSetSensitive( menu_telewin_file,   sort_connected );
    XtSetSensitive( menu_telewin_show,   sort_connected );
    XtSetSensitive( menu_sort_show,      sort_connected );
    
    if( sort_connected ) {
        int sb, se;
        float sal;
        m_sort_get_buffers(sb, se, sal);

        char buf[12];

        snprintf(buf, sizeof(buf), "%8d", sb);
        XmTextSetString(text_sorting_buffers, buf);

        snprintf(buf, sizeof(buf), "%8d", se);
        XmTextSetString(text_sorting_errors, buf);

        snprintf(buf, sizeof(buf), "%.3f", sal);
        XmTextSetString(text_sorting_avelen, buf);
    } else {
        XmTextSetString(text_sorting_buffers, ccc("<n.c.>"));
        XmTextSetString(text_sorting_errors, ccc("<n.c.>"));
        XmTextSetString(text_sorting_avelen, ccc("<n.c.>"));
    }

    if( engine_connected )
        engine_check_directory();
    if( sort_connected )
        sort_check_directory();
}

// ########################################################################

typedef void (*filename_callback)(const std::string& filename);

static void show_filename_callback(Widget widget, XtPointer callback, XtPointer call_data)
{
    XmSelectionBoxCallbackStruct *cbs = (XmSelectionBoxCallbackStruct *)call_data;
    char *fname = 0;
    if (!XmStringGetLtoR (cbs->value, XmFONTLIST_DEFAULT_TAG, &fname))
        return; /* must have been an internal error */
    const std::string filename(fname);
    XtFree(fname);
    XtDestroyWidget(widget);

    if( callback )
        ((filename_callback)callback)(filename);
}

// ########################################################################

static void dialog_fix_size(Widget dialog)
{
    XtWidgetGeometry size;
    XtQueryGeometry(dialog, 0, &size);
    XtVaSetValues(XtParent(dialog),
                  XmNminWidth,  size.width,
                  XmNmaxWidth,  size.width,
                  XmNminHeight, size.height,
                  XmNmaxHeight, size.height,
                  NULL);
}

// ########################################################################

static void show_filename_dialog(const char* prompt, const char* title,
                                 filename_callback callback, const std::string& deflt)
{
    XmString xdeflt  = XmStringCreateLocalized(ccc(deflt.c_str()));
    XmString xprompt = XmStringCreateLocalized(ccc(prompt));
    XmString xtitle  = XmStringCreateLocalized(ccc(title));
    Widget dialog = XmCreatePromptDialog (toplevel, ccc("FilenameDialog"), NULL, 0);
    XtVaSetValues(dialog,
                  XmNselectionLabelString, xprompt,
                  XmNautoUnmanage,         False,
                  XmNdialogStyle,          XmDIALOG_FULL_APPLICATION_MODAL,
                  XmNdialogTitle,          xtitle,
                  XmNtextString,           xdeflt,
                  NULL);
    XmStringFree(xdeflt ); // always destroy compound strings when done
    XmStringFree(xprompt);
    XmStringFree(xtitle );

    dialog_fix_size(dialog);

    XtAddCallback (dialog, XmNokCallback, show_filename_callback, (void*)callback);
    XtAddCallback (dialog, XmNcancelCallback, (XtCallbackProc)XtDestroyWidget, NULL);

    XtUnmanageChild (XtNameToWidget (dialog, "Help"));
    XtManageChild (dialog);
    XtPopup (XtParent (dialog), XtGrabNone);
}

// ########################################################################

static void exit_prog(Widget, XtPointer, XtPointer)
{
    XtAppSetExitFlag(app_context);
}

// ########################################################################

static void connect_engine_pushed(Widget, XtPointer, XtPointer)
{
    extern void connect_to_engine();
    connect_to_engine();
}

// ########################################################################

static void connect_sort_pushed(Widget, XtPointer, XtPointer)
{
    extern void connect_to_sort();
    connect_to_sort();
}

// ########################################################################

static void quit_pushed(Widget, XtPointer, XtPointer)
{
    Widget ex_dialog = XmCreateQuestionDialog(toplevel, ccc("QuitConfirmation"), NULL, 0);

    XmString title  = XmStringCreateLocalized(ccc("SIRIUS - Confirm Quit"));
    XmString yes = XmStringCreateLocalized(ccc("Yes"));
    XmString no  = XmStringCreateLocalized(ccc("No"));
    XmString msg = XmStringCreateLocalized(ccc("Are you sure you want to quit SIRIUS?\n"
                                               "This will not stop engine or sort."));
    XtVaSetValues( ex_dialog,
                   XmNdialogStyle,       XmDIALOG_FULL_APPLICATION_MODAL,
                   XmNdialogTitle,       title,
                   XmNmessageString,     msg,
                   XmNokLabelString,     yes,
                   XmNcancelLabelString, no,
                   NULL );
    XmStringFree(title);
    XmStringFree(yes  );
    XmStringFree(no   );
    XmStringFree(msg  );

    dialog_fix_size(ex_dialog);

    XtAddCallback (ex_dialog, XmNcancelCallback, (XtCallbackProc)XtDestroyWidget, NULL);
    XtAddCallback (ex_dialog, XmNokCallback, exit_prog, 0);

    XtSetSensitive ( XmMessageBoxGetChild (ex_dialog, XmDIALOG_HELP_BUTTON), False);
    XtUnmanageChild( XmMessageBoxGetChild (ex_dialog, XmDIALOG_HELP_BUTTON));
    XtManageChild (ex_dialog);

    XtPopup (XtParent (ex_dialog), XtGrabNone);
}

// ########################################################################

static void start_pushed(Widget, XtPointer, XtPointer)
{
    if( !m_engine_is_started() )
        acq_start();
    else
        log_message(LOG_ERR, "engine already started !\n");
}

// ########################################################################

static void stop_pushed(Widget, XtPointer, XtPointer)
{
    if( m_engine_is_started() )
        acq_stop();
    else
        log_message(LOG_ERR, "engine not started !\n");
}

// ########################################################################

static void status_pushed(Widget, XtPointer, XtPointer)
{
    if( m_engine_is_connected() )
        acq_status( );
    else
        log_message(LOG_ERR, "engine not connected !\n");
}


// ########################################################################

static void reload_GUI(Widget, XtPointer, XtPointer)
{
    if( m_engine_is_connected() ){
	if ( !m_engine_is_started() )
        	acq_reload( );
	else
		log_message(LOG_ERR, "engine is running !\n");
    } else
        log_message(LOG_ERR, "engine not connected !\n");
}


// ########################################################################

static void clear_pushed(Widget, XtPointer, XtPointer)
{
    if( m_sort_is_connected() )
        acq_clear();
    else
        log_message(LOG_ERR, "sort not connected !\n");
}

// ########################################################################

void dump_pushed(Widget, XtPointer, XtPointer)
{
    if( m_sort_is_connected() )
        acq_dump();
    else
        log_message(LOG_ERR, "sort not connected !\n");
}

// ########################################################################

// void list_pushed(Widget, XtPointer, XtPointer)
// {
//     const int buflength = 1024;
//     int       i;
//     char      buf[buflength], c;
// 
//     FILE *speclist = fopen( "/Applications/sirius/help/spectra.list","r" );
//     for(i=0; i<buflength-1 && (c = getc(speclist)) && c != EOF; i++)
//         buf[i] = toascii(c);
//     fclose( speclist );
//     buf[i] = '\0';
// 
//     log_message(LOG_INFO, "%s", buf);
// }

// ########################################################################

static void mama_pushed(Widget, XtPointer, XtPointer)
{
    if( !commands->run("mama") )
        log_message(LOG_ERR, "Could not run 'mama' command.\n");
    else
        log_message(LOG_INFO, "Started mama (it might not be found, though).\n");
}

// ########################################################################

static void rupdate_pushed(Widget, XtPointer, XtPointer)
{
    if( !commands->run("rupdate") )
        log_message(LOG_ERR, "Could not start 'rupdate'.\n");
    else
        log_message(LOG_INFO, "Started 'rupdate'.\n");
}

// ########################################################################

static void none_device_pushed(Widget, XtPointer, XtPointer)
{
    if( m_engine_is_connected() )
        acq_storage( false );
    else
        log_message(LOG_ERR, "engine not connected !\n");
}

// ########################################################################

static void disc_device_pushed(Widget, XtPointer, XtPointer)
{
    if( m_engine_is_connected() )
        acq_storage( true );
    else
        log_message(LOG_ERR, "engine not connected !\n");
}

// ########################################################################

static void change_pushed(Widget, XtPointer, XtPointer)
{
    if( !m_engine_is_connected() )
        log_message(LOG_ERR, "engine not connected !\n");
    else if( !m_engine_get_output() )
        log_message(LOG_ERR, "Cannot change 'none' output.\n");
    else
        acq_storage(true);
}

// ########################################################################

static void gaininit_pushed(Widget, XtPointer, XtPointer)
{
    if( !m_sort_is_connected() )
        log_message(LOG_ERR, "sort not connected !\n");
    else
        m_sort_gainshift("/Applications/sirius/data/gainshift.init");
}

// ########################################################################

static void gainshift_get_file(const std::string& filename)
{
    if( filename.empty() ) {
        log_message(LOG_ERR, "Empty filename, no new gain/shift.\n");
        return;
    }

    last_gain_filename = filename;
    std::ifstream infile(filename.c_str());
    if( !infile.is_open() ) {
        log_message(LOG_ERR, "could not open '%s' for reading, no new gain/shift.\n",
                 filename.c_str());
        return;
    }

#if 0
    const char* outfile_name = "/Applications/sirius/data/gainshift.tmp";
    std::ofstream outfile( outfile_name );
    if( !outfile.is_open() ) {
        log_message(LOG_ERR, "could not open '%s' for writing, no new gain/shift.\n",
                 outfile_name);
        return;
    }

    outfile << infile.rdbuf();

    m_sort_gainshift(outfile_name);
#else
    m_sort_gainshift(filename.c_str());
#endif
}

// ########################################################################

static void gainfile_pushed(Widget, XtPointer, XtPointer)
{
    show_filename_dialog("Gain/Shift File", "Gain/shift file name:",
                         gainshift_get_file, last_gain_filename);
}

// ########################################################################

static void gainshow_pushed(Widget, XtPointer, XtPointer)
{
    if( !m_sort_is_connected() )
        log_message(LOG_ERR, "sort not connected !\n");
    else
        m_sort_status_gain();
}

// ########################################################################

static void telewininit_pushed(Widget, XtPointer, XtPointer)
{
    if( !m_sort_is_connected() )
        log_message(LOG_ERR, "sort not connected !\n");
    else
        m_sort_telewin("/Applications/sirius/data/telewin.init");
}

// ########################################################################

static void telewin_get_file(const std::string& filename)
{
    if( filename.empty() ) {
        log_message(LOG_ERR, "Empty filename, no new telewin data.\n");
        return;
    }

    last_telewin_filename = filename;
    std::ifstream infile(filename.c_str());
    if( !infile.is_open() ) {
        log_message(LOG_ERR, "could not open '%s' for reading, no new telewin data.\n",
                 filename.c_str());
        return;
    }
#if 0
    const char* outfile_name = "/Applications/sirius/data/telewin.tmp";
    std::ofstream outfile( outfile_name );
    if( !outfile.is_open() ) {
        log_message(LOG_ERR, "could not open '%s' for writing, no new telewin data.\n",
                 outfile_name);
        return;
    }

    outfile << infile.rdbuf();

    m_sort_telewin(outfile_name);
#else
    m_sort_telewin(filename.c_str());
#endif
}

// ########################################################################

static void telewinfile_pushed(Widget, XtPointer, XtPointer)
{
    show_filename_dialog("Telewin file name:", "Telewin File",
                         telewin_get_file, last_telewin_filename);
}

// ########################################################################

static void telewinshow_pushed(Widget, XtPointer, XtPointer)
{
    if( !m_sort_is_connected() )
        log_message(LOG_ERR, "sort not connected !\n");
    else
        m_sort_status_telewin();
}

// ########################################################################

static void connect_sort_timeout(XtPointer, XtIntervalId*)
{
    extern void connect_to_sort();
    connect_to_sort();
}

// ########################################################################

static void sortfunc_get_file(const std::string& filename)
{
    if( filename.empty() ) {
        log_message(LOG_ERR, "Empty filename, no new sort function.\n");
        return;
    }

    last_sort_filename = filename;

    if( !commands->run("loadsort", filename) )
        log_message(LOG_ERR, "Could not start loadsort.\n");

    // add timer to automatically reconnect to sort -- here only, or
    // also generallly when sort is not connected?
    XtAppAddTimeOut(app_context, 1500 /* ms */, connect_sort_timeout, 0);
}

// ########################################################################

static void load_pushed(Widget, XtPointer, XtPointer)
{
    show_filename_dialog("Sorting function file name:", "Sorting Function",
                         sortfunc_get_file, last_sort_filename);
}

// ########################################################################

static void show_pushed(Widget, XtPointer, XtPointer)
{
    if( !m_sort_is_connected() )
        log_message(LOG_ERR, "sort not connected !\n");
    else
        m_sort_status_user();
}

// ########################################################################

static void h_about_pushed(Widget, XtPointer, XtPointer)
{
    XmString text = XmStringCreateLtoR (ccc("      Welcome to SIRIUS 2.0\n"
                                            "\n"
                                            "Oslo Cyclotron Laboratory\n"
                                            "         Data Acquisiton System\n"
                                            "\n"
                                            "tore.ramsoy@nrpa.no\n"
                                            "  alexander.buerger@fys.uio.no\n"
                                            "    magne.guttormsen@fys.uio.no\n"
                                            "            Oslo September 2011\n"),
                                        XmFONTLIST_DEFAULT_TAG);
    XmString title  = XmStringCreateLocalized(ccc("About SIRIUS"));

    Widget dialog = XmCreateInformationDialog(toplevel, ccc("AboutDialog"), NULL, 0);
    XtVaSetValues(dialog,
                  XmNdialogStyle,   XmDIALOG_FULL_APPLICATION_MODAL,
                  XmNdialogTitle,   title,
                  XmNmessageString, text,
                  NULL);
    XmStringFree(text );
    XmStringFree(title);

    XtUnmanageChild ( XmMessageBoxGetChild( dialog, XmDIALOG_CANCEL_BUTTON) );
    XtUnmanageChild ( XmMessageBoxGetChild( dialog, XmDIALOG_HELP_BUTTON) );
    /*XtUnmanageChild ( XmMessageBoxGetChild( dialog, XmDIALOG_SYMBOL_LABEL) ); */

    dialog_fix_size(dialog);

    XtManageChild( dialog );
    XtPopup(XtParent(dialog), XtGrabNone);
}

// ########################################################################

static void h_install_pushed(Widget, XtPointer, XtPointer)
{
    if( !commands->run("readme") )
        log_message(LOG_ERR, "Could not run 'readme' command.\n");
    else
        log_message(LOG_INFO, "Started command to show the README file.\n");
}

// ########################################################################

static void h_online_pushed(Widget, XtPointer, XtPointer)
{
    if( !commands->run("manual") )
        log_message(LOG_ERR, "Could not run 'manual' command.\n");
}

// ########################################################################

static void clear_info_pushed(Widget, XtPointer, XtPointer)
{
    XmTextSetString(text_infos, ccc(""));
}

// ########################################################################

static void clear_errors_pushed(Widget, XtPointer, XtPointer)
{
    XmTextSetString(text_errors, ccc(""));
}

// ########################################################################
// ########################################################################
// ########################################################################

static Widget create_pulldown(Widget menubar, const char* name, const char* label, char mnemo)
{
    Widget PullDown = XmCreatePulldownMenu(menubar, ccc(name), NULL, 0);

    XmString l_label = XmStringCreateLocalized(ccc(label));
    XtVaCreateManagedWidget(label,
                            xmCascadeButtonWidgetClass, menubar,
                            XmNlabelString,             l_label,
                            XmNmnemonic,                mnemo,
                            XmNsubMenuId,               PullDown,
                            NULL);
    XmStringFree( l_label );

    return PullDown;
}

// ########################################################################

static Widget create_item(Widget pulldown, const char* label,
                          XtCallbackProc callback, XtPointer client_data=0)
{
    Widget item = XtVaCreateManagedWidget
        ( label, xmPushButtonGadgetClass, pulldown, NULL);
    XtAddCallback(item, XmNactivateCallback, callback, client_data);

    return item;
}

// ########################################################################

static Widget create_separator(Widget parent)
{
    return XtVaCreateManagedWidget("Separator", xmSeparatorGadgetClass, parent, NULL);
}

// ########################################################################

static Widget gui_setup_menubar(Widget parent)
{
    Widget MenuBar = XmCreateMenuBar(parent, ccc("MenuBar"), NULL, 0);

    // ------------------------------------------------------------------
    // create the "File" menu
    Widget FilePullDown = create_pulldown(MenuBar, "MenuFile", "File", 'F');

    menu_file_engine = create_item(FilePullDown, "Connect to engine", connect_engine_pushed );
    menu_file_sort   = create_item(FilePullDown, "Connect to sort",   connect_sort_pushed  );

    // add the menu item QUIT
    XmString quit_accel_text = XmStringCreateLocalized(ccc("Ctrl+Q"));
    Widget menu_file_quit = create_item(FilePullDown, "Quit", quit_pushed);
    XtVaSetValues(menu_file_quit,
                  XmNacceleratorText, quit_accel_text,
                  XmNaccelerator,     "Ctrl<Key>Q",
                  NULL);
    XmStringFree( quit_accel_text );

    // ------------------------------------------------------------------
    // create the "Run" menu
    Widget RunPullDown = create_pulldown(MenuBar, "MenuRun", "Run", 'R');

    menu_run_start  = create_item(RunPullDown, "Start",  start_pushed );
    menu_run_stop   = create_item(RunPullDown, "Stop",   stop_pushed  );
    create_separator(RunPullDown);
    menu_run_status = create_item(RunPullDown, "Status", status_pushed);
    menu_run_GUI = create_item(RunPullDown, "Launch GUI", reload_GUI);

    // ------------------------------------------------------------------
    // create the "Spectra" menu
    Widget SpecPullDown = create_pulldown(MenuBar, "MenuSpectra", "Spectra", 'S');
    XtVaSetValues(SpecPullDown, XmNtearOffModel, XmTEAR_OFF_ENABLED, NULL);

    menu_spectra_clear = create_item(SpecPullDown, "Clear", clear_pushed);
    menu_spectra_dump  = create_item(SpecPullDown, "Dump",  dump_pushed );
    // spec_list =       create_item(SpecPullDown, "List",  list_pushed );
    create_separator(SpecPullDown);
    /* spectra_mama */   create_item(SpecPullDown, "MAMA",  mama_pushed );
    /* spectra_rupdate */create_item(SpecPullDown, "ROOTupdate",  rupdate_pushed );

    // ------------------------------------------------------------------
    // create the "Output" menu
    Widget OutputPullDown = create_pulldown(MenuBar,"MenuOutput", "Output", 'O');

    /* Add the menu DEVICE */
    Widget DevicePullRight = XmCreatePulldownMenu
        ( OutputPullDown, ccc("DevicePullRight"), NULL, 0);

    menu_output_device = XtVaCreateManagedWidget
        ("Device", xmCascadeButtonGadgetClass, OutputPullDown,
         XmNsubMenuId, DevicePullRight, NULL);

    Widget device_none = XtVaCreateManagedWidget
        ("None",   xmPushButtonGadgetClass, DevicePullRight, NULL);
    Widget device_disc = XtVaCreateManagedWidget
        ("Disk",   xmPushButtonGadgetClass, DevicePullRight, NULL);

    XtAddCallback(device_none, XmNactivateCallback, none_device_pushed, NULL);
    XtAddCallback(device_disc, XmNactivateCallback, disc_device_pushed, NULL);

    menu_output_change = create_item(OutputPullDown, "Change file", change_pushed);

    // ------------------------------------------------------------------
    // create the "Gain" menu
    Widget GainPullDown = create_pulldown(MenuBar, "MenuGain", "Gain", 'G');

    menu_gain_init = create_item(GainPullDown, "Initialize",    gaininit_pushed);
    menu_gain_file = create_item(GainPullDown, "From File ...", gainfile_pushed);
    create_separator(GainPullDown);
    menu_gain_show = create_item(GainPullDown, "Show values",   gainshow_pushed);

    // ------------------------------------------------------------------
    // Create the "TeleWin" menu
    Widget TeleWinPullDown = create_pulldown(MenuBar, "MenuTelewin", "TeleWin", 'T');

    menu_telewin_init = create_item(TeleWinPullDown, "Initialize",    telewininit_pushed);
    menu_telewin_file = create_item(TeleWinPullDown, "From File ...", telewinfile_pushed);
    create_separator(TeleWinPullDown);
    menu_telewin_show = create_item(TeleWinPullDown, "Show values",   telewinshow_pushed);

    // ------------------------------------------------------------------
    // create the "Sorting" pulldown menu
    Widget SortPullDown = create_pulldown(MenuBar, "MenuSort", "SortFunc", 'o');

    menu_sort_load = create_item(SortPullDown, "Load ...", load_pushed);
    menu_sort_show = create_item(SortPullDown, "Show",     show_pushed);

    // ------------------------------------------------------------------
    // create the "Help" menu
    Widget HelpPullDown = create_pulldown(MenuBar, "MenuHelp", "Help", 'H');
    XtVaSetValues(MenuBar, XmNmenuHelpWidget, HelpPullDown, NULL); // postition HELP to right

    create_item(HelpPullDown, "About Sirius", h_about_pushed);
    create_item(HelpPullDown, "Install",      h_install_pushed);
    create_separator(HelpPullDown);
    create_item(HelpPullDown, "Sirius",       h_online_pushed);

    XtManageChild (MenuBar);
    return MenuBar;
}

// ########################################################################

static Widget create_button(Widget parent, const char* name, const char* label,
                          XtCallbackProc callback, XtPointer client_data=0)
{
    XmString l_label = XmStringCreateLocalized( ccc(label) );
    Widget button = XtVaCreateManagedWidget
        (name, xmPushButtonWidgetClass, parent, XmNlabelString, l_label, NULL);
    XmStringFree( l_label );

    XtVaSetValues(button, XmNalignment, XmALIGNMENT_CENTER, NULL);
    XtAddCallback(button, XmNactivateCallback, callback, client_data);

    return button;
}

// ########################################################################

static Widget gui_setup_buttonbar(Widget parent)
{
    // ------------------------------------------------------------------
    // Add a row of pushbuttons below the menubar
    Widget buttons = XtVaCreateManagedWidget
        ("ButtonRow", xmRowColumnWidgetClass, parent, NULL);
    XtVaSetValues( buttons,
                   XmNpacking,     XmPACK_COLUMN,
                   XmNnumColumns,  1,
                   XmNorientation, XmHORIZONTAL,
                   NULL);

    button_start  = create_button(buttons, "ButtonStart",  "Start",  start_pushed);
    button_stop   = create_button(buttons, "ButtonStop",   "Stop",   stop_pushed);
    button_status = create_button(buttons, "ButtonStatus", "Status", status_pushed);

    button_change_file = create_button(buttons, "ButtonChangeFile", "Change File", change_pushed);

    return buttons;
}

// ########################################################################

static const char* fallback_resources[] = {
    "Sirius*Foreground:                  black",
    "Sirius.MainForm.SeparatorOutput.topOffset:   0",
    "Sirius.MainForm.FormOutput.topOffset:        2",
    "Sirius.MainForm.FormOutput.leftOffset:       2",
    "Sirius.MainForm.FormOutput.rightOffset:      2",
    "Sirius.MainForm.FormOutput2.topOffset:       2",
    "Sirius.MainForm.FormOutput2.leftOffset:      2",
    "Sirius.MainForm.FormOutput2.rightOffset:     2",
    "Sirius.MainForm.SeparatorSorting.topOffset:  2",
    "Sirius.MainForm.FormSorting.topOffset:       2",
    "Sirius.MainForm.FormSorting.leftOffset:      2",
    "Sirius.MainForm.FormSorting.rightOffset:     2",
    "Sirius.MainForm.SeparatorMessages.topOffset: 2",
    "Sirius.MainForm.PaneMessages.topOffset:      0",
    "Sirius*Background:                  #e02010",
    "Sirius*highlightColor:              #901810",
    "Sirius*XmText*Background:           #cccccc",
    "Sirius*TextInfo*Foreground:         MidnightBlue",
    "Sirius*TextErrors*Foreground:       Red",
    NULL
};

int gui_setup(int& argc, char* argv[])
{
    // build the gui, start with motif top level routines
    XtSetLanguageProc (NULL, NULL, NULL);
    toplevel = XtVaAppInitialize
        (&app_context, "Sirius", NULL, 0, &argc, argv, /*applicationShellWidgetClass,*/
         const_cast<char**>(fallback_resources),
         XmNdeleteResponse, XmDO_NOTHING,
         NULL);


    // ------------------------------------------------------------------
    // define a 'Form' layout manager as child of toplevel
    Widget form = XtVaCreateManagedWidget
        ("MainForm", xmFormWidgetClass, toplevel, NULL);

    // ------------------------------------------------------------------
    // create the menu bar
    Widget MenuBar = gui_setup_menubar(form);
    XtVaSetValues(MenuBar,
                  XmNtopAttachment,   XmATTACH_FORM,
                  XmNleftAttachment,  XmATTACH_FORM,
                  XmNrightAttachment, XmATTACH_FORM,
                  NULL);

    // ------------------------------------------------------------------
    // create the button bar
    Widget buttonbar = gui_setup_buttonbar(form);
    XtVaSetValues(buttonbar,
                  XmNtopAttachment,   XmATTACH_WIDGET,
                  XmNtopWidget,       MenuBar,
                  XmNleftAttachment,  XmATTACH_FORM,
                  NULL);

    // ------------------------------------------------------------------
    // "Output" row:

    Widget separator_output = XtVaCreateManagedWidget
        ("SeparatorOutput", xmSeparatorGadgetClass, form,
         XmNtopAttachment,   XmATTACH_WIDGET,
         XmNtopWidget,       buttonbar,
         XmNleftAttachment,  XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         NULL);

    Widget output_form = XtVaCreateManagedWidget
        ("FormOutput", xmFormWidgetClass, form, 
         XmNtopAttachment,   XmATTACH_WIDGET,
         XmNtopWidget,       separator_output,
         XmNleftAttachment,  XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         NULL);
    
    Widget label_output = XtVaCreateManagedWidget
        ("Output buffers: ", xmLabelGadgetClass, output_form,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNleftAttachment,   XmATTACH_FORM,
         NULL);

    text_output_buffers = XtVaCreateManagedWidget
        ("TextOutputBuffers", xmTextWidgetClass, output_form,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNleftAttachment,   XmATTACH_WIDGET,
         XmNleftWidget,       label_output,
         XmNeditable,         FALSE,
         XmNeditMode,         XmSINGLE_LINE_EDIT,
         XmNcolumns,          8,
         NULL);

    Widget label_output_2 = XtVaCreateManagedWidget
        (" File: ", xmLabelGadgetClass, output_form,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNleftAttachment,   XmATTACH_WIDGET,
         XmNleftWidget,       text_output_buffers,
         NULL);

    text_output_file = XtVaCreateManagedWidget
        ("TextOutputFilename", xmTextWidgetClass, output_form,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNleftAttachment,   XmATTACH_WIDGET,
         XmNleftWidget,       label_output_2,
         XmNrightAttachment,  XmATTACH_FORM,
         XmNeditMode,         XmSINGLE_LINE_EDIT,
         XmNcolumns,          30,
         XmNeditable,         FALSE,
         NULL);

    Widget output_form_2 = XtVaCreateManagedWidget
        ("FormOutput2", xmFormWidgetClass, form, 
         XmNtopAttachment,   XmATTACH_WIDGET,
         XmNtopWidget,       output_form,
         XmNleftAttachment,  XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         NULL);
    
    Widget label_output_3 = XtVaCreateManagedWidget
        ("Buffer rate [1/s]: ", xmLabelGadgetClass, output_form_2,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNleftAttachment,   XmATTACH_FORM,
         XmNleftOffset,       5,
         NULL);

    text_output_bufrate = XtVaCreateManagedWidget
        ("TextOutputBufRate", xmTextWidgetClass, output_form_2,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNleftAttachment,   XmATTACH_WIDGET,
         XmNleftWidget,       label_output_3,
         XmNeditMode,         XmSINGLE_LINE_EDIT,
         XmNcolumns,          6,
         XmNeditable,         FALSE,
         NULL);

    // ------------------------------------------------------------------
    // "Sorting" row:

    Widget separator_sorting = XtVaCreateManagedWidget
        ("SeparatorSorting", xmSeparatorGadgetClass, form,
         XmNtopAttachment,   XmATTACH_WIDGET,
         XmNtopWidget,       output_form_2,
         XmNleftAttachment,  XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         NULL);

    Widget sorting_form = XtVaCreateManagedWidget
        ("FormSorting", xmFormWidgetClass, form, 
         XmNtopAttachment,   XmATTACH_WIDGET,
         XmNtopWidget,       separator_sorting,
         XmNleftAttachment,  XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         NULL);
    
    Widget label_sorting = XtVaCreateManagedWidget
        ("Sorted buffers: ", xmLabelGadgetClass, sorting_form,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNleftAttachment,   XmATTACH_FORM,
         NULL);

    text_sorting_buffers = XtVaCreateManagedWidget
        ("TextSortingBuffers", xmTextWidgetClass, sorting_form,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNleftAttachment,   XmATTACH_WIDGET,
         XmNleftWidget,       label_sorting,
         XmNeditable,         FALSE,
         XmNeditMode,         XmSINGLE_LINE_EDIT,
         XmNcolumns,          8,
         NULL);

    Widget label_sorting_2 = XtVaCreateManagedWidget
        (" Errors: ", xmLabelGadgetClass, sorting_form,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNleftAttachment,   XmATTACH_WIDGET,
         XmNleftWidget,       text_sorting_buffers,
         NULL);

    text_sorting_errors = XtVaCreateManagedWidget
        ("TextSortingErrors", xmTextWidgetClass, sorting_form,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNleftAttachment,   XmATTACH_WIDGET,
         XmNleftWidget,       label_sorting_2,
         XmNeditMode,         XmSINGLE_LINE_EDIT,
         XmNcolumns,          8,
         XmNeditable,         FALSE,
         NULL);

    Widget sorting_form_2 = XtVaCreateManagedWidget
        ("FormSorting2", xmFormWidgetClass, form, 
         XmNtopAttachment,   XmATTACH_WIDGET,
         XmNtopWidget,       sorting_form,
         XmNleftAttachment,  XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         NULL);
    
    Widget label_sorting_3 = XtVaCreateManagedWidget
        ("Avg. length: ", xmLabelGadgetClass, sorting_form_2,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNleftAttachment,   XmATTACH_FORM,
         XmNleftOffset,       20,
         NULL);

    text_sorting_avelen = XtVaCreateManagedWidget
        ("TextSortingAvelen", xmTextWidgetClass, sorting_form_2,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         XmNleftAttachment,   XmATTACH_WIDGET,
         XmNleftWidget,       label_sorting_3,
         XmNeditMode,         XmSINGLE_LINE_EDIT,
         XmNcolumns,          6,
         XmNeditable,         FALSE,
         NULL);

    // ------------------------------------------------------------------
    // paned window to allow adjusting the height of info an error
    // windows
    Widget separator_messages = XtVaCreateManagedWidget
        ("SeparatorMessages", xmSeparatorGadgetClass, form,
         XmNtopAttachment,   XmATTACH_WIDGET,
         XmNtopWidget,       sorting_form_2,
         XmNleftAttachment,  XmATTACH_FORM,
         XmNrightAttachment, XmATTACH_FORM,
         NULL);

    Widget paned_window = XtVaCreateWidget("PaneMessages", xmPanedWindowWidgetClass, form,
         XmNtopAttachment,    XmATTACH_WIDGET,
         XmNtopWidget,        separator_messages,
         XmNleftAttachment,   XmATTACH_FORM,
         XmNrightAttachment,  XmATTACH_FORM,
         XmNbottomAttachment, XmATTACH_FORM,
         NULL);

    // ------------------------------------------------------------------
    // info pane
    Widget info_form = XtVaCreateManagedWidget
        ("FormInfo", xmFormWidgetClass, paned_window,
         XmNpaneMinimum, 50,
         NULL);

    // button to clear info messages
    Widget button_clear_info = create_button(info_form, "ButtonClearInfo",
                                             "Clear", clear_info_pushed);
    XtVaSetValues(button_clear_info,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNrightAttachment,  XmATTACH_FORM,
         NULL);

    // create a text widget for program output
    text_infos = XmCreateScrolledText(info_form, ccc("TextInfo"), NULL, 0);
    XtVaSetValues(XtParent(text_infos), // have to use parent, because of scrollbars
                  XmNtopAttachment,    XmATTACH_WIDGET,
                  XmNtopWidget,        button_clear_info,
                  XmNleftAttachment,   XmATTACH_FORM,
                  XmNrightAttachment,  XmATTACH_FORM,
                  XmNbottomAttachment, XmATTACH_FORM,
                  NULL);
    XtVaSetValues(text_infos,
                  XmNrows,             10,
                  XmNcolumns,          70,
                  XmNeditable,         FALSE,
                  XmNeditMode,         XmMULTI_LINE_EDIT,
                  XmNscrollHorizontal, FALSE,
                  XmNwordWrap,         TRUE,
                  NULL);
    XtManageChild (text_infos);

    // label of message text widget
    XtVaCreateManagedWidget
        ("Messages:", xmLabelGadgetClass, info_form,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget,     XtParent(text_infos),
         XmNleftAttachment,   XmATTACH_FORM,
         NULL);


    // ------------------------------------------------------------------
    // errors pane
    Widget errors_form = XtVaCreateManagedWidget
        ("FormErrors", xmFormWidgetClass, paned_window,
         XmNpaneMinimum, 50,
         NULL);

    // button to clear error messages
    Widget button_clear_errors = create_button(errors_form, "ButtonClearErrors",
                                               "Clear", clear_errors_pushed);
    XtVaSetValues(button_clear_errors,
         XmNtopAttachment,    XmATTACH_FORM,
         XmNrightAttachment,  XmATTACH_FORM,
         NULL);

    // text widget for error output
    text_errors = XmCreateScrolledText(errors_form, ccc("TextErrors"), NULL, 0);
    XtVaSetValues(XtParent(text_errors), // have to use parent, because of scrollbars
                  XmNtopAttachment,     XmATTACH_WIDGET,
                  XmNtopWidget,         button_clear_errors,
                  XmNleftAttachment,    XmATTACH_FORM,
                  XmNrightAttachment,   XmATTACH_FORM,
                  XmNbottomAttachment,  XmATTACH_FORM,
                  NULL);
    XtVaSetValues(text_errors,
                  XmNrows,              5,
                  XmNcolumns,           70,
                  XmNeditable,          FALSE,
                  XmNeditMode,          XmMULTI_LINE_EDIT,
                  XmNscrollHorizontal,  FALSE,
                  XmNwordWrap,          TRUE,
                  NULL);
    XtManageChild (text_errors);

    // label of error text widget
    XtVaCreateManagedWidget
        ("Errors:", xmLabelGadgetClass, errors_form,
         XmNbottomAttachment, XmATTACH_WIDGET,
         XmNbottomWidget,     XtParent(text_errors),
         XmNleftAttachment,   XmATTACH_FORM,
         NULL);

    // ------------------------------------------------------------------
    XtManageChild (paned_window);

    XtManageChild (form);

    gui_update_state();

    XtRealizeWidget (toplevel);

    XtWidgetGeometry size;
    XtQueryGeometry(form, 0, &size);
    XtVaSetValues(toplevel,
                  XmNminWidth,  size.width,
                  XmNmaxWidth,  2*size.width,
                  XmNminHeight, size.height,
                  XmNmaxHeight, 2*size.height,
                  NULL);

    Atom AtWM_PROTOCOLS     = XmInternAtom(XtDisplay(toplevel), ccc("WM_PROTOCOLS"), False);
    Atom AtWM_DELETE_WINDOW = XmInternAtom(XtDisplay(toplevel), ccc("WM_DELETE_WINDOW"), False);
    XmAddProtocols(toplevel, AtWM_PROTOCOLS, &AtWM_DELETE_WINDOW, 1);
    XmAddProtocolCallback(toplevel, AtWM_PROTOCOLS, AtWM_DELETE_WINDOW, quit_pushed, NULL);

    return 0;
}

// ########################################################################

void logbook_message(std::string const& subject, std::string const& message)
{
    std::vector<std::string> args;
    args.push_back("-a");
    args.push_back(std::string("Subject=")+subject);
    args.push_back(message);
    commands->run("elog", args);
}

// ########################################################################

void log_message(int level, const char *fmt, ...)
{
    if( (level>LOG_ERR && !text_infos) || (level==LOG_ERR && !text_errors) )
        return;

    char msgbuf[8192];
    va_list ap;
    va_start (ap, fmt);
    (void) vsnprintf(msgbuf, sizeof(msgbuf), fmt, ap);
    va_end (ap);

    Widget& text = (level>LOG_ERR) ? text_infos : text_errors;

    time_t now = time(0);
    char timebuf[128];
    strftime(timebuf, sizeof(timebuf), "%Y-%m-%d %H:%M:%S: ", localtime(&now));

    XmTextPosition pos = XmTextGetLastPosition(text);
    XmTextInsert(text, pos, timebuf);

    pos = XmTextGetLastPosition(text);
    XmTextInsert(text, pos, msgbuf);

#if 0
    if( level==LOG_ERR ) {
        std::string msg = "Error message in acquisition: ";
        msg += msgbuf;
        logbook_message("Error message in ACQ", msg);
    }
#endif

    pos = XmTextGetLastPosition(text);
    XtVaSetValues(text, XmNcursorPosition, pos, NULL);
    XmTextShowPosition(text, pos);
}

/*
  TODO:

  add semaphore for shm-spectra writing

  show manual in non-modal app window (text widget), not nedit
  => manual describes installation, I think it is not very useful
  to call it from the running progam
 */
