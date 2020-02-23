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
    
    while(1) {
        // Update display only if there are changes
        if(g_SConfig.bUpdateDisp) {
            g_SConfig.bUpdateDisp = FALSE;
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
            mvwprintw(stdscr, wY++, wX+4, "%s", g_SConfig.szLastCmdBuff);

            UM34C_prettyPrintData(&g_SConfig.pSUM34C_config->SCurrentData, TRUE);

            wrefresh(stdscr);
        }

        usleep(INTREVAL_UPDATE_DISPLAY);  // sleep time should be the as long as timer interval
    }
}