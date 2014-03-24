#include <ncurses.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

#define f(x,y) for (int x = 0; x < y; ++x)
#define ROW 1000
#define COL 1000

int adx[9] = {-1, 0, 1, -1, 0, 1, -1, 0, 1}, ady[9] = {1, 1, 1, 0, 0, 0, -1, -1, -1};
int row, col, px, py, ch;
bool rock[ROW][COL];
bool bold[ROW][COL];
char filechar[ROW][COL];
bool from_file;

void initialize() {
	initscr();
	noecho();
	keypad(stdscr, TRUE);
	cbreak();
	start_color();
	init_pair(1, COLOR_YELLOW, COLOR_BLACK);
	init_pair(2, COLOR_GREEN, COLOR_BLACK);
	curs_set(FALSE);
	srand((unsigned) time(NULL));
	
	row = 20, col = 60;
	px = col/2, py = row/2;
	f(i,row) f(j,col) if (i == 0 || i == row-1 || j == 0 || j == col-1 || rand()%13 < 4) rock[i][j] = true;
	f(i,row) f(j,col) if (rand()%3 == 0) bold[i][j] = true;
}

void moveplayer(int dir) {
	if ((px+adx[dir] >= 0) && (py+ady[dir] >= 0) && (((px+adx[dir] < col) && (py+ady[dir] < row)) || (from_file && (px+adx[dir] < COL) && (py+ady[dir] < ROW)))) {
		if (!rock[py+ady[dir]][px+adx[dir]]) {
			px += adx[dir];
			py += ady[dir];
		}
	}
}

void draw (int cx, int cy, int dis) {
	if (!((cx >= 0) && (cy >= 0) && (((cx < col) && (cy < row)) || (from_file && (cx < COL) && (cy < ROW))))) return;
	if ((cx-px)*(cx-px) + (cy-py)*(cy-py) <= dis*dis + 1) {	// circular view - can be changed if you like
		if (!from_file) {
			if (rock[cy][cx]) {
				if (bold[cy][cx]) mvaddch(cy, cx, '*' | A_BOLD | COLOR_PAIR(2));
				else mvaddch(cy, cx, '*' | COLOR_PAIR(2));
			} else mvaddch(cy, cx, '.' | COLOR_PAIR(1));
		} else if (cx-px+col/2 >= 0 && cx-px+col/2 <= col && cy-py+row/2 >= 0 && cy-py+row/2 <= row) {
			mvaddch(cy-py+row/2, cx-px+col/2, filechar[cy][cx]);
		}
	}
}

int ccw(int x1, int y1, int x2, int y2, int x3, int y3) {	// positive iff they are counterclockwise
	return (x1*y2 + x2*y3 + x3*y1 - x1*y3 - x2*y1 - x3*y2);
}

// permissive field of view
void trace_pfov(int dir, int n, int h) {
	int topx[n+h+2], topy[n+h+2], botx[n+h+2], boty[n+h+2];	// convex hull of obstructions
	int curt = 0, curb = 0;	// size of top and bottom convex hulls
	int s[2][2] = {{0, 0}, {0, 0}};	// too lazy to think of real variable names, four critical points on the convex hulls - these four points determine what is visible
	topx[0] = boty[0] = 0, topy[0] = botx[0] = 1;
	for (int ad1[2] = {0, 0}, ad2[2] = {0, 0}, eps[2] = {0, n+h-1}; ad1[0]+ad2[0] <= n+h;) {
		f(i,2) {
			eps[i] += h;	// good old Bresenham... modified a bit...
			if (eps[i] >= n+h) {
				eps[i] -= n+h;
				++ad2[i];
			} else ++ad1[i];
		}
		f(i,2) if (ccw(topx[s[!i][1]], topy[s[!i][1]], botx[s[i][0]], boty[s[i][0]], ad1[i]+1-i, ad2[i]+i) <= 0) return;	// the relevant region is no longer visible. If we don't exit the loop now, strange things happen.
		f(i,2) if (ccw(topx[s[!i][1]], topy[s[!i][1]], botx[s[i][0]], boty[s[i][0]], i, 1-i) == 0 && s[0][i] != 0) return;	// Those pesky corner cases. The line is exactly touching a corner of our square...
		int cx[2] = {ad1[0], ad1[1]}, cy[2] = {ad2[0], ad2[1]};
		f(i,2) {
			if (dir&1) cx[i] = -cx[i];
			if (dir&2) cy[i] = -cy[i];
			cx[i] += px, cy[i] += py;
			
			if (ccw(topx[s[i][1]], topy[s[i][1]], botx[s[!i][0]], boty[s[!i][0]], ad1[i]+i, ad2[i]+1-i) > 0) {
				draw(cx[i], cy[i], ((n>h)?n:h));
			}
		}
		
		if (cx[0] < 0 || cy[0] < 0 || cx[0] > COL || cy[0] > ROW || rock[cy[0]][cx[0]]) {	// new obstacle, update convex hull
			++curb;
			botx[curb] = ad1[0], boty[curb] = ad2[0]+1;
			if (ccw(botx[s[0][0]], boty[s[0][0]], topx[s[1][1]], topy[s[1][1]], ad1[0], ad2[0]+1) > 0) return;	// the obstacle obscures everything
			if (ccw(topx[s[0][1]], topy[s[0][1]], botx[s[1][0]], boty[s[1][0]], ad1[0], ad2[0]+1) >= 0) {
				s[1][0] = curb;	// updating visible region
				while (s[0][1] < curt && ccw(topx[s[0][1]], topy[s[0][1]], topx[s[0][1]+1], topy[s[0][1]+1], ad1[0], ad2[0]+1) > 0) ++s[0][1];
			}
			while (curb > 1 && ccw(botx[curb-2], boty[curb-2], botx[curb-1], boty[curb-1], ad1[0], ad2[0]+1) >= 0) {	// not convex anymore, delete a point
				if (s[1][0] == curb) --s[1][0];
				if (s[0][0] == curb) --s[0][0];
				--curb;
				botx[curb] = botx[curb+1], boty[curb] = boty[curb+1];
			}
		}
		
		if (cx[1] < 0 || cy[1] < 0 || cx[1] > COL || cy[1] > ROW || rock[cy[1]][cx[1]]) {	// same as above
			++curt;
			topx[curt] = ad1[1]+1, topy[curt] = ad2[1];
			if (ccw(botx[s[1][0]], boty[s[1][0]], topx[s[0][1]], topy[s[0][1]], ad1[1]+1, ad2[1]) > 0) return;
			if (ccw(topx[s[1][1]], topy[s[1][1]], botx[s[0][0]], boty[s[0][0]], ad1[1]+1, ad2[1]) >= 0) {
				s[1][1] = curt;
				while (s[0][0] < curb && ccw(botx[s[0][0]], boty[s[0][0]], botx[s[0][0]+1], boty[s[0][0]+1], ad1[1]+1, ad2[1]) < 0) ++s[0][0];
			}
			while (curt > 1 && ccw(topx[curt-2], topy[curt-2], topx[curt-1], topy[curt-1], ad1[1]+1, ad2[1]) <= 0) {
				if (s[1][1] == curt) --s[1][1];
				if (s[0][1] == curt) --s[0][1];
				--curt;
				topx[curt] = topx[curt+1], topy[curt] = topy[curt+1];
			}
		}
	}
}

int distance;

void showgame() {
	erase();
	
	f(dir, 4) f(i,distance+1) trace_pfov(dir, distance, i), trace_pfov(dir, i, distance);
	
	if (!from_file) mvaddch(py, px, '@' | A_BOLD);
	else mvaddch(row/2, col/2, '@' | A_BOLD);
	
	mvprintw(row+1, 0, "Numpad moves, 'q' quits.");
	refresh();
}

void playgame() {
	while (1) {
		showgame();
		ch = getch();
		if (ch == 'q') break;
		if (ch >= '1' && ch <= '9') moveplayer(ch-'1');
	}
}

void cleanup() {
	endwin();
}

void readfile(char *filepath) {
	f(i,ROW) f(j,COL) rock[i][j] = false;
	FILE *fin = fopen(filepath, "r");
	if (fin == NULL) return;
	int curx = 0, cury = 0;
	while (!feof(fin) && curx < COL && cury < ROW) {
		char c = fgetc(fin);
		if (c == '\n') {
			++cury, curx = 0;
			continue;
		}
		if (c == '#') rock[cury][curx] = true;
		if (c == '@') px = curx, py = cury;
		filechar[cury][curx] = c;
		++curx;
	}
}

int main(int argc, char **argv) {
	distance = 10;
	if (argc > 1) distance = atoi(argv[1]);
	
	initialize();
	
	if (argc > 2) {	// input from file!
		readfile(argv[2]);
		from_file = true;
	} else from_file = false;
	
	playgame();
	
	cleanup();
	
	return 0;
}
