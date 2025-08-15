#ifndef XIACONTROL_H
#define XIACONTROL_H

// Header for containers
#include <queue>
#include <vector>
#include <deque>
#include <string>
#include <map>
#include <functional>

#include <sys/time.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

class WriteTerminal;

#define XIA_MIN_READOUT 65536
#define XIA_FIFO_MIN_READOUT 2048
#define SIRIUS_BUFFER_SIZE 32768
#define SCALER_FILE_NAME_IN "scalers_in.dat"
#define SCALER_FILE_NAME_OUT "scalers_out.dat"
#define SCALER_FILE_CSV "scalers.csv"
#define PRESET_MAX_MODULES 24  // Maximum number of modules. As in the XIA API
#if REMOVE_TRACE // Set at compile time!
    #define MAX_RAWDATA_LEN 18 // Maximum length of a raw event with trace
#else
    #define MAX_RAWDATA_LEN 4114
#endif // REMOVE_TRACE

typedef struct {
    int64_t timestamp;                  //! Timestamp of the event.
    uint32_t raw_data[MAX_RAWDATA_LEN]; //! Pointer to the raw data.
    int size_raw;                       //! Size of the raw data in number of 32 bit words.
} Event_t;
inline bool operator>(const Event_t &a, const Event_t &b) { return (a.timestamp>b.timestamp); }

class XIAControl
{
private:

    // Object responsible for I/O to the stdout & stderr
    WriteTerminal *termWrite;

    // Ordered queue to fill with data as it arrives
    //std::vector<Event_t> sorted_events;
    std::priority_queue<Event_t, std::vector<Event_t>, std::greater<Event_t> > sorted_events;

    // Number of 32-bit words in the queue
    int data_avalible;

    // Most recent timestamp of each module
    int64_t most_recent_t[PRESET_MAX_MODULES];

    // Temporary storage for data that are trapped between two buffers.
    std::vector<uint32_t> overflow_queue;

    // Temporary storage for data that are trapped between two buffers.
    std::vector<uint32_t> overflow_fifo[PRESET_MAX_MODULES];

    // Flag to indicate that modules are initialized.
    bool is_initialized;

    // Flag to indicate that modules are booted.
    bool is_booted;

    // Flag to indicate that a run is active
    bool is_running;

    // Filename of the current XIA DSP settings file.
    std::string settings_file;

    // Number of modules
    int num_modules;

    // PXI mapping of the modules.
    unsigned short PXISlotMap[PRESET_MAX_MODULES];

    // Timestamps needs to be multiplied by either 8 or 10 to
    // get timestamp in ns. We will set these values when we
    // read the sampling frequency from the modules.
    // 250 MHz -> 8 ns
    // 500 MHz -> 10 ns
    int timestamp_factor[PRESET_MAX_MODULES];

    std::string firmware_config;

    // Temporary buffer for strings
    char errmsg[1024];

    // Last 'stats' read out.
    unsigned int last_stats[PRESET_MAX_MODULES][448];

    timeval last_time;

    // raw memory used during readout of list mode.
    unsigned int *lmdata;

public:

    XIAControl(WriteTerminal *writeTerm,
               const unsigned short PXImap[PRESET_MAX_MODULES], /*!< PXI mapping                */
               const std::string &FWname="XIA_Firmware.txt",    /*!< Path to firmware settings  */
               const std::string &SETname="settings.set",       /*!< Path to settings file      */
               const bool& bootmode = 0);


    // Destructor. Quite important for this
    // class, as it will release all resorces
    // related to the XIA modules and make sure
    // the thread is terminated.
    ~XIAControl();

    // Set internal flag to indicate that the thread should exit.
    void Terminate(){ }

    // Poll to check if we have enough data to fill a buffer
    // (note, this will be false until we have 65536 32-bit words of data)
    bool XIA_check_buffer(int bufsize);

    // Ask for a buffer of data to be committed.
    bool XIA_fetch_buffer(uint32_t *buffer, int bufsize, unsigned int *first_header);

    // Ask the class to boot the XIA modules.
    bool boot();

    // Ask the class to start the run in the XIA modules.
    bool XIA_start_run();

    // Check the run status
    bool XIA_check_status();

    // Ask the class to end the run in the XIA modules.
    // We will also ask for the file where data are stored as we
    // will need to flush all data to this file.
    bool XIA_end_run(FILE *output_file=NULL, /* Ending the run will flush all remaning data to file */
                     const char *fname = "");

    // Ask for a reload.
    // Will return false if run is active.
    bool XIA_reload();

    // Get number of XIA modules.
    inline int GetNumMod() const { return num_modules; }

private:

    // Some private functions that are needed.

    // Function to initialize the XIA modules.
    bool InitializeXIA(const bool &offline = false);

    // Function to boot the XIA modules.
    bool BootXIA();

    // Function to adjust the baseline.
    bool AdjustBaseline();

    // Function to adjust baseline cut.
    bool AdjustBlCut();

    // Function to start the actual list mode run.
    bool StartLMR();

    // Function where we do the actual stop of a run.
    bool StopRun();

    // Function to check if there is a run in progress.
    // Return false if no run is in progress, false otherwise.
    bool CheckIsRunning();

    // Function to synch all the modules.
    bool SynchModules();

    // Function to read and write scalers.
    bool WriteScalers();

    // Function checking if FIFO has enough data to force a readout.
    bool CheckFIFO(unsigned int minReadout=16384);

    // Read FIFO of all modules and commit data to the queue.
    bool ReadFIFO();

    // Release all XIA resorces.
    bool ExitXIA();

    // Parse data and commit to the queue.
    void ParseQueue(uint32_t *raw_data, size_t size, int module);
};

#endif // XIACONTROL_H
