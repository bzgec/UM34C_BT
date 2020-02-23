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


void displayHelp(uint16_t *wY, uint16_t *wX) {
    /* 
    * Exit application: `CTRL+C` or `c`
    * Set device brightness: `UP`/`DOWN`
    * Move to previous/next display: `LEFT`/`RIGHT`
    * Toggle data sampling: `SPACE`
    * Toggle saving to CSV file: `s`
    * Rotate screen: `r`
    * Set screen timeout: `0`-`9`
    */
    mvwprintw(stdscr, (*wY)++, *wX, "Exit application: `CTRL+C` or `c`");
    mvwprintw(stdscr, (*wY)++, *wX, "Set device brightness: `UP`/`DOWN`");
    mvwprintw(stdscr, (*wY)++, *wX, "Move to previous/next display: `LEFT`/`RIGHT`");
    mvwprintw(stdscr, (*wY)++, *wX, "Toggle data sampling: `SPACE`");
    mvwprintw(stdscr, (*wY)++, *wX, "Toggle saving to CSV file: `s`");
    mvwprintw(stdscr, (*wY)++, *wX, "Rotate screen: `r`");
    mvwprintw(stdscr, (*wY)++, *wX, "Set screen timeout: `0`-`9`");
}

// http://www.cs.ukzn.ac.za/~hughm/os/notes/ncurses.html
void *threadDisplayStuff(void *arg) {
    initscr(); // Initialize the window
    noecho(); // Don't echo any keypresses
    // curs_set(FALSE); // Don't display a cursor
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
    char szLastCmdBuff[40] = "No command yet";
    
    while(1) {
        wX = 0;
        wY = 0;

        wclear(stdscr); // Clear the screen of all

        // Display help at the top
        displayHelp(&wY, &wX);

        // Display other information a bit lower
        if(g_SConfig.pSFileHandler_config->bFileCreated_CSV) {
            wY += 4;  // display file size 
        } else {
            wY += 5;  // do not display file size if there is no file
        }

        mvwprintw(stdscr, wY, wX, "Reading data: ");
        displayTrueFalse(wY++, wX+14, g_SConfig.bReadData);
        mvwprintw(stdscr, wY, wX, "Saving to CSV file: ");
        displayTrueFalse(wY++, wX+20, g_SConfig.bSaveToCSVfile);

        if(g_SConfig.pSFileHandler_config->bFileCreated_CSV) {
            mvwprintw(stdscr, wY++, wX, "File size: %0.1f kB", getFileSize(g_SConfig.pSFileHandler_config->szCSVfileName, fileHandler_size_kB));
        }

        mvwprintw(stdscr, wY++, wX, "Last command: ");
        mvwprintw(stdscr, wY++, wX+4, "%s", szLastCmdBuff);

        UM34C_prettyPrintData(&g_SConfig.pSUM34C_config->SCurrentData, TRUE);

        if(g_SConfig.nCmdChar != g_SConfig.nCmdChar_prev) {
            if(g_SConfig.nCmdChar == ERR) {
                // no input from terminal
            } else {
                switch (g_SConfig.nCmdChar) {
                case KEY_UP:  // Brightness UP
                    g_SConfig.byDeviceBrightness = g_SConfig.pSUM34C_config->SCurrentData.byBrightness;
                    if(g_SConfig.byDeviceBrightness < um34c_cmd_setBrightness5-um34c_cmd_setBrightness0) {
                        g_SConfig.byDeviceBrightness++;
                    }
                    UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, (um34c_cmd_E)(g_SConfig.byDeviceBrightness + um34c_cmd_setBrightness0), &g_SConfig.pSUM34C_config->nStatus);
                    sprintf(szLastCmdBuff, "Brightness: %d (UP)", g_SConfig.byDeviceBrightness);
                    break;
                case KEY_DOWN:  // Brightness DOWN
                    g_SConfig.byDeviceBrightness = g_SConfig.pSUM34C_config->SCurrentData.byBrightness;
                    if(g_SConfig.byDeviceBrightness > um34c_cmd_setBrightness0-um34c_cmd_setBrightness0) {
                        g_SConfig.byDeviceBrightness--;
                    }
                    UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, (um34c_cmd_E)(g_SConfig.byDeviceBrightness + um34c_cmd_setBrightness0), &g_SConfig.pSUM34C_config->nStatus);
                    sprintf(szLastCmdBuff, "Brightness: %d (DOWN)", g_SConfig.byDeviceBrightness);
                    break;
                case KEY_LEFT:  // Previous display
                    UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, um34c_cmd_prev, &g_SConfig.pSUM34C_config->nStatus);
                    sprintf(szLastCmdBuff, "Previous display");
                   break;
                case KEY_RIGHT:  // Next display
                    UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, um34c_cmd_next, &g_SConfig.pSUM34C_config->nStatus);
                    sprintf(szLastCmdBuff, "Next display");
                    break;
                case 'c':  // Exit program
                    exitProgram(exitProgram_param_C);
                    break;
                case 0x20:  // Toggle data sampling - 'space'
                    g_SConfig.bReadData = !g_SConfig.bReadData;
                    sprintf(szLastCmdBuff, "Toggle data sampling.");
                    break;
                case 's':  // Toggle saving to CSV file
                    g_SConfig.bSaveToCSVfile = !g_SConfig.bSaveToCSVfile;
                    sprintf(szLastCmdBuff, "Toggle saving to CSV file.");
                    break;
                case 'r':  // rotate screen
                    UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, um34c_cmd_rotateClockwise, &g_SConfig.pSUM34C_config->nStatus);
                    g_SConfig.byCurrentScreenRotation++;
                    if(g_SConfig.byCurrentScreenRotation % 4 == 0) {
                        g_SConfig.byCurrentScreenRotation = 0;
                    }
                    sprintf(szLastCmdBuff, "Rotating screen: %d", g_SConfig.byCurrentScreenRotation);
                    break;
                case '0':  // set screen timeout
                case '1':
                case '2':
                case '3':
                case '4':
                case '5':
                case '6':
                case '7':
                case '8':
                case '9':
                    UM34C_sendCmd(g_SConfig.pSUM34C_config->nSocketHandle, (g_SConfig.nCmdChar - '0') + um34c_cmd_setTimeout0, &g_SConfig.pSUM34C_config->nStatus);
                    sprintf(szLastCmdBuff, "Screen timeout: %d", g_SConfig.nCmdChar - '0');
                    break;
                default:
                    break;
                }
            }
            g_SConfig.nCmdChar = ERR;  // so that we can press 'UP' two times...
            g_SConfig.nCmdChar_prev = g_SConfig.nCmdChar;
            logger(log_lvl_debug, "display", "Last command: %s", szLastCmdBuff);
        }

        wrefresh(stdscr);

        usleep(INTREVAL_UPDATE_DISPLAY);  // sleep time should be the as long as timer interval
    }
}