
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdlib.h>
#ifdef _WIN32
#include <windows.h>
#define sleep_ms(ms) Sleep(ms)
#else
#include <unistd.h>
#define sleep_ms(ms) usleep((ms) * 1000)
#endif

#ifdef _WIN32
static void enable_ansi_virtual_terminal() {
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut == INVALID_HANDLE_VALUE) return;
	DWORD dwMode = 0;
	if (!GetConsoleMode(hOut, &dwMode)) return;
	dwMode |= ENABLE_VIRTUAL_TERMINAL_PROCESSING;
	SetConsoleMode(hOut, dwMode);
}
#endif

#ifndef _WIN32
#include <sys/ioctl.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#endif

static void get_terminal_size(int *cols, int *rows) {
#ifdef _WIN32
	CONSOLE_SCREEN_BUFFER_INFO csbi;
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	if (hOut && GetConsoleScreenBufferInfo(hOut, &csbi)) {
		*cols = csbi.srWindow.Right - csbi.srWindow.Left + 1;
		*rows = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
		return;
	}
	*cols = 80; *rows = 22;
#else
	struct winsize ws;
	if (ioctl(1, TIOCGWINSZ, &ws) == 0) {
		*cols = ws.ws_col; *rows = ws.ws_row;
		return;
	}
	*cols = 80; *rows = 22;
#endif
}

int main(int argc, char** argv) {
	float A = 0.0f, B = 0.0f;
	int maxFrames = 0;
	if (argc > 1) maxFrames = atoi(argv[1]);
	int frames = 0;

	// enable ANSI on Windows terminals that support it
#ifdef _WIN32
	enable_ansi_virtual_terminal();
#endif

	int width, height;
	get_terminal_size(&width, &height);
	if (width < 40) width = 80;
	if (height < 20) height = 22;
	int size = width * height;

	char *b = malloc(size);
	float *z = malloc(sizeof(float) * size);

	for (;;) {
		if (!b || !z) break;
		memset(b, ' ', size);
		for (int i = 0; i < size; ++i) z[i] = 0.0f;
		memset(b, ' ', sizeof(b));
		for (int i = 0; i < size; ++i) z[i] = 0.0f;

		for (float j = 0.0f; j < 6.28f; j += 0.07f) {
			for (float i = 0.0f; i < 6.28f; i += 0.02f) {
				float c = sinf(i);
				float d = cosf(j);
				float e = sinf(A);
				float f = sinf(j);
				float g = cosf(A);
				float h = d + 2.0f;
				float D = 1.0f / (c * h * e + f * g + 5.0f);
				float l = cosf(i);
				float m = cosf(B);
				float n = sinf(B);
				float t = c * h * g - f * e;
				int x = (int)(width/2 + 30.0f * D * (l * h * m - t * n));
				int y = (int)(height/2 + 15.0f * D * (l * h * n + t * m));
				int o = x + width * y;
				int N = (int)(8.0f * ((f * e - c * h * g) * m - c * h * e - f * g - l * h * n));
				if (y >= 0 && y < height && x >= 0 && x < width && D > z[o]) {
					z[o] = D;
					static const char* shades = ".,-~:;=!*#$@";
					int idx = N > 0 ? N : 0;
					if (idx >= 0 && idx < (int)strlen(shades)) b[o] = shades[idx];
				}
			}
		}

		// Clear screen and move cursor to home, then draw
	#ifdef _WIN32
		// If ANSI wasn't enabled, clearing via system is a fallback
		printf("\x1b[2J\x1b[H");
	#else
		printf("\x1b[2J\x1b[H");
	#endif
		for (int k = 0; k < size; ++k) {
		    putchar(b[k]);
		    if ((k + 1) % width == 0) putchar('\n');
		}
		fflush(stdout);

		A += 0.04f;
		B += 0.02f;
		sleep_ms(30);

		if (maxFrames && ++frames >= maxFrames) break;
	}

	free(b);
	free(z);
	return 0;
}
