#include "display.h"
#include "config.h"
#include <ncurses.h>
#include "main.h"  // exitProgram()
#include <unistd.h>
#include "logger.h"


extern mainConfig_S g_SConfig;


void displayTrueFalse(uint16_t wY, uint16_t wX, uint8_t bDispTrue) {
    if(bDispTrue == FALSE) {
        attron(COLOR_PAIR(RED_PAIR) | A_BOLD);
        mvwprintw(stdscr, wY, wX, "FALSE");
        attroff(COLOR_PAIR(RED_PAIR) | A_BOLD);
    } else {
        attron(COLOR_PAIR(GREEN_PAIR) | A_BOLD);
        mvwprintw(stdscr, wY, wX, "TRUE");
        attroff(COLOR_PAIR(GREEN_PAIR) | A_BOLD);
    }
}


void displayHelp(uint16_t *wY, uint16_t *wX, uint8_t bUseNcurses) {
    /* 
    * Press `h` to display/close help
    * Exit application: `CTRL+C` or `c`
    * Move to previous/next display: `LEFT`/`RIGHT`
    * Toggle data sampling: `SPACE`
    * Toggle saving to CSV file: `s`
    * Rotate screen: `r`
    * Set device brightness:
    *   1. First press `b`
    *   2. Than change brightness with `UP`/`DOWN`
    * Set record current threshold (when recording (mAh and mWh) starts):
    *   1. First press `i`
    *   2. Than change threshold with `UP`/`DOWN`
    * Set screen timeout (`0` is OFF, `9` is 9 min):
    *   1. First press `t`
    *   2. Than select timeout with `0`-`9`
    * Set data group:
    *   1. First press `g`
    *   2. Than select data group with `0`-`9`
    * 
    */

    if(bUseNcurses) {
        mvwprintw(stdscr, (*wY)++, *wX, "Press `h` to close help");
        mvwprintw(stdscr, (*wY)++, *wX, "Exit application: `CTRL+C` or `c`");
        mvwprintw(stdscr, (*wY)++, *wX, "Move to previous/next display: `LEFT`/`RIGHT`");
        mvwprintw(stdscr, (*wY)++, *wX, "Toggle data sampling: `SPACE`");
        mvwprintw(stdscr, (*wY)++, *wX, "Toggle saving to CSV file: `s`");
        mvwprintw(stdscr, (*wY)++, *wX, "Rotate screen: `r`");
        mvwprintw(stdscr, (*wY)++, *wX, "Set device brightness:");
        mvwprintw(stdscr, (*wY)++, *wX + 4, "1. First press `b`");
        mvwprintw(stdscr, (*wY)++, *wX + 4, "2. Than change brightness with `UP`/`DOWN`");
        mvwprintw(stdscr, (*wY)++, *wX, "Set record current threshold (when recording (mAh and mWh) starts):");
        mvwprintw(stdscr, (*wY)++, *wX + 4, "1. First press `i`");
        mvwprintw(stdscr, (*wY)++, *wX + 4, "2. Than change threshold with `UP`/`DOWN`");
        mvwprintw(stdscr, (*wY)++, *wX, "Set screen timeout (`0` is OFF, `9` is 9 min):");
        mvwprintw(stdscr, (*wY)++, *wX + 4, "1. First press `t`");
        mvwprintw(stdscr, (*wY)++, *wX + 4, "2. Than select timeout with `0`-`9`");
        mvwprintw(stdscr, (*wY)++, *wX, "Set data group:");
        mvwprintw(stdscr, (*wY)++, *wX + 4, "1. First press `g`");
        mvwprintw(stdscr, (*wY)++, *wX + 4, "2. Than select data group with `0`-`9`");  
    } else {
        printf("Running application (this options are optional):\n\r");
        printf("\t- Specify UM34C bluetooth address:\n\r\t\t`build/um34c -a 00:15:A3:00:2D:BF` \n\r");
        printf("\t- Set interval at which data from UM34C is queried (in ms):\n\r\t\t`build/um34c -q 1000` \n\r");
        printf("\t- Start reading data from UM34C at startup:\n\r\t\t`build/um34c -r` \n\r");
        printf("\t- Start reading and saving data from UM34C at startup:\n\r\t\t`build/um34c -r -csv` \n\r");
        printf("\t- Append average from X samples of voltage and current to CSV file\n\r\t  " \
               "(in this case every 20 samples average is appended to CSV file and\n\r\t  " \
               "becasue we set query interval to 500ms every 10 seconds value is\n\r\t  " \
               "appended to CSV file):\n\r\t\t`build/um34c -f 20 -q 500` \n\r");
        printf("\t- Set record current threshold in cA (at which current measuring\n\r\t  " \
               "mAh and mWh starts). Here we set it to 5cA which is 0.05A:\n\r\t\t`build/um34c -i 5`\n\r");
        printf("\t- If GUI is not used (for example program runned in background)\n\r\t  " \
               "use `-n` flag, example of programm running in background:\n\r\t\t`build/um34c -f 4 -q 500 -r -csv -i 5 -n > /dev/null &`\n\r" \
               "\t   - To kill program like that run `ps aux | grep um34c`\n\r\t     " \
               "and than `kill yourPID` where 'yourPID' is PID you got\n\r\t     " \
               "from command before.");
        printf("\n\rIn application:\n\r");
        printf("\t- Press `h` to display/close help");
        printf("\t- Exit application: `CTRL+C` or `c`\n\r");
        printf("\t- Move to previous/next display: `LEFT`/`RIGHT`\n\r");
        printf("\t- Toggle data sampling: `SPACE`\n\r");
        printf("\t- Toggle saving to CSV file: `s`\n\r");
        printf("\t- Rotate screen: `r`\n\r");
        printf("\t- Set device brightness:\n\r");
        printf("\t\t1. First press `b`\n\r");
        printf("\t\t2. Than change brightness with `UP`/`DOWN`\n\r");
        printf("\t- Set record current threshold (when recording (mAh and mWh) starts):\n\r");
        printf("\t\t1. First press `i`\n\r");
        printf("\t\t2. Than change threshold with `UP`/`DOWN`\n\r");
        printf("\t- Set screen timeout (`0` is OFF, `9` is 9 min):\n\r");
        printf("\t\t1. First press `t`\n\r");
        printf("\t\t2. Than select timeout with `0`-`9`\n\r");
        printf("\t- Set data group:\n\r");
        printf("\t\t1. First press `g`\n\r");
        printf("\t\t2. Than select data group with `0`-`9`\n\r");  
    }
}

// http://www.cs.ukzn.ac.za/~hughm/os/notes/ncurses.html
void *threadDisplayStuff(void *arg) {
    initscr(); // Initialize the window
    noecho(); // Don't echo any keypresses
    curs_set(FALSE); // Don't display a cursor
    keypad(stdscr, TRUE);  // To capture special keystrokes like Backspace, Delete and the four arrow keys by getch()
    cbreak();  // no key buffering
    // nodelay(stdscr, TRUE);  // non-blocking getch()
    nodelay(stdscr, FALSE);  // blocking getch()

    if (has_colors() == FALSE) {
        endwin();  // dellocates memory and ends ncurses
        printf("Your terminal does not support color\n\r");
        logger(log_lvl_error, "display", "Your terminal does not support color");
        exitProgram(exitProgram_param_terminalColor);
    } else {
        use_default_colors();
        start_color();
        init_pair(GREEN_PAIR, COLOR_GREEN, DEFAULT_CLR);
        init_pair(RED_PAIR, COLOR_RED, DEFAULT_CLR);
    }

    uint16_t wX;
    uint16_t wY;
    
    while(1) {
        // Update display only if there are changes
        if(g_SConfig.bUpdateDisp) {
            g_SConfig.bUpdateDisp = FALSE;
            wX = 0;
            wY = 0;

            wclear(stdscr); // Clear the screen of all

            if(g_SConfig.bShowHelp) {
                // Display only help
                displayHelp(&wY, &wX, TRUE);
            } else {
                // Print on the left side of screen
                UM34C_prettyPrintData(&wY, &wX, &g_SConfig.pSUM34C_config->SCurrentData, TRUE);
                wX=0;

                wY++;

                mvwprintw(stdscr, wY, wX, "Reading data: ");
                displayTrueFalse(wY++, wX+14, g_SConfig.bReadData);
                mvwprintw(stdscr, wY, wX, "Saving to CSV file: ");
                displayTrueFalse(wY++, wX+20, g_SConfig.bSaveToCSVfile);

                if(g_SConfig.pSFileHandler_config->bFileCreated_CSV) {
                    mvwprintw(stdscr, wY++, wX, "File size: %0.1f kB", BYTES_TO_kB(getFileSize(g_SConfig.pSFileHandler_config->szCSVfileName)));
                }

                mvwprintw(stdscr, wY++, wX, "Last command: ");
                mvwprintw(stdscr, wY++, wX+4, "%s", g_SConfig.szLastCmdBuff);

                // Display everythin and data display help hint
                mvwprintw(stdscr, wY+2, wX, "Press `h` to display help");

                // Print on the right side of screen
                wY = 0;
                wX = 40;
                UM34C_prettyPrintSettings(&wY, &wX, &g_SConfig.pSUM34C_config->SCurrentData, TRUE);
                wX = 40;

                wY += 2;

                mvwprintw(stdscr, wY++, wX, "Query data interval: %ums", g_SConfig.pSUM34C_config->dwTimerInterval/1000);
                if(g_SConfig.wMovAvgStrength != 0) {
                    // Moving average filter is used
                    mvwprintw(stdscr, wY++, wX, "Filter strength: %u", g_SConfig.wMovAvgStrength);
                    mvwprintw(stdscr, wY++, wX, "Appending to CSV file every: %0.1fs", g_SConfig.wMovAvgStrength*(g_SConfig.pSUM34C_config->dwTimerInterval/1000.0/1000.0));
                } else {
                    // Moving average filter is NOT used
                    mvwprintw(stdscr, wY++, wX, "Appending to CSV file every: %0.1fs", g_SConfig.pSUM34C_config->dwTimerInterval/1000.0/1000.0);
                   wY++;
                }

                // // Display other information a bit lower
                // if(g_SConfig.pSFileHandler_config->bFileCreated_CSV) {
                //     wY += 1;  // display file size 
                // } else {
                //     wY += 2;  // do not display file size if there is no file
                // }

            }
            
            wrefresh(stdscr);
        }

        usleep(INTREVAL_UPDATE_DISPLAY);  // sleep time should be the as long as timer interval
    }
}