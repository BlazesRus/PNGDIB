/* smview - PNG Image Viewer Demo
 * A really basic PNG image viewer for windows.
 * This program is a demonstration of the pngdib library.
 * It also requires the libpng and zlib libraries (needed by pngdib).
 *
 * By Jason Summers
 *
 * Copyright is disclaimed on the source code in this file.
 */

#include <windows.h>
#include <tchar.h>
#include <commdlg.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include "resource.h"
#include <pngdib.h>
#include <strsafe.h>

#define MYNAME _T("Simple PNG Image Viewer")
#define SMVIEW_VERS  _T("3.1.0")
#define SMVIEW_DATE  _T("7/2010")

struct globals_struct {
	HINSTANCE hInst;
	COLORREF custcolors[16];
	int use_bkgd, use_custom_bg;
	int use_gamma;
	COLORREF custom_bg, image_bg;
	int which_bg;   /* 0==custom or default (depending on use_custom_bg) */
					/* 1==image's */
};
static struct globals_struct *g;

struct img_struct {
	LPBITMAPINFOHEADER dib;
	void *bits;
	TCHAR png_filename[MAX_PATH];
};
static struct img_struct *img;

static LRESULT CALLBACK WndProcMain(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);
static INT_PTR CALLBACK DlgProcAbout(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam);

static int choose_color_dialog(HWND hwnd, COLORREF *color)
{
	CHOOSECOLOR cc;

	ZeroMemory(&cc,sizeof(CHOOSECOLOR));
	cc.lStructSize=sizeof(CHOOSECOLOR);
	cc.hwndOwner=hwnd;
	cc.rgbResult= *color;  /* set initial value */
	cc.lpCustColors=g->custcolors;
	cc.Flags=CC_RGBINIT|CC_FULLOPEN;

	if(ChooseColor(&cc)) {
		(*color) = cc.rgbResult;
		return 1;
	}
	return 0;
}

static int open_png_by_name(HWND hwnd, const TCHAR *fn)
{
	PNGDIB *pngdib;
	TCHAR buf[1000];
	int ret;
	unsigned char bg_r, bg_g, bg_b;
	int dibsize, bits_offset;

	if(!lstrlen(fn)) {
		InvalidateRect(hwnd,NULL,TRUE);
		return 0;
	}

	pngdib = pngdib_p2d_init();
	if(!pngdib) return 0;

	pngdib_p2d_set_png_filename(pngdib,fn);
	pngdib_p2d_set_use_file_bg(pngdib,g->use_bkgd?1:0);
	if(g->use_custom_bg) {
		pngdib_p2d_set_custom_bg(pngdib,GetRValue(g->custom_bg),
			GetGValue(g->custom_bg),GetBValue(g->custom_bg));
	}
	pngdib_p2d_set_gamma_correction(pngdib,g->use_gamma,PNGDIB_DEFAULT_SCREEN_GAMMA);

	ret=pngdib_p2d_run(pngdib);

	if(ret) {    /* returns 0 on success */
		SetForegroundWindow(hwnd);
		StringCbPrintf(buf,sizeof(buf),_T("Cannot load image: %s (%d)"),pngdib_get_error_msg(pngdib),ret);
		MessageBox(hwnd,buf,MYNAME,MB_OK|MB_ICONWARNING);
		InvalidateRect(hwnd,NULL,TRUE);
		pngdib_done(pngdib);
		return 0;
	}

	/* get rid of previous image */
	if(img->dib) { pngdib_p2d_free_dib(NULL,img->dib); img->dib=NULL; }

	pngdib_p2d_get_dib(pngdib,&img->dib,&dibsize);
	pngdib_p2d_get_dibbits(pngdib,&img->bits,&bits_offset,NULL);

	InvalidateRect(hwnd,NULL,TRUE);
	StringCchCopy(img->png_filename,MAX_PATH,fn);

	if(pngdib_p2d_get_bgcolor(pngdib,&bg_r,&bg_g,&bg_b)) {
		g->image_bg = RGB(bg_r,bg_g,bg_b);
		g->which_bg=1;
	}
	else {
		g->which_bg=0;
	}

	pngdib_done(pngdib);

	return 1;
}

static int open_png_dialog(HWND hwnd)
{
	TCHAR fn[MAX_PATH];
	OPENFILENAME ofn;

	StringCbCopy(fn,sizeof(fn),_T(""));

	ZeroMemory(&ofn,sizeof(OPENFILENAME));

	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=hwnd;
	ofn.lpstrFilter=_T("PNG image (*.png)\0*.png\0All files\0*.*\0\0");
	ofn.nFilterIndex=1;
	ofn.lpstrTitle=_T("Open PNG file");
	
	ofn.lpstrInitialDir=NULL;
	ofn.lpstrFile=fn;
	ofn.nMaxFile=MAX_PATH;
	ofn.Flags=OFN_FILEMUSTEXIST|OFN_HIDEREADONLY;

	if(!GetOpenFileName(&ofn)) return 0;  /* canceled by user */
	return open_png_by_name(hwnd,fn);
}

static void dropped_files(HWND hwnd, HDROP hDrop)
{
	UINT ret;
	TCHAR fn[MAX_PATH];

	ret=DragQueryFile(hDrop,0xFFFFFFFF,NULL,0);
	if(ret==1) {  	/* number of files dropped */
		ret=DragQueryFile(hDrop,0,fn,MAX_PATH);
		if(ret) open_png_by_name(hwnd,fn);
	}
	DragFinish(hDrop);
}

static void save_as_png(HWND hwnd)
{
	PNGDIB *pngdib;
	int ret;
	OPENFILENAME ofn;
	TCHAR fn[MAX_PATH];
	TCHAR buf[200];

	StringCbCopy(fn,sizeof(fn),img->png_filename);

	ZeroMemory(&ofn,sizeof(OPENFILENAME));
	ofn.lStructSize=sizeof(OPENFILENAME);
	ofn.hwndOwner=hwnd;
	ofn.lpstrFilter=_T("PNG image (*.png)\0*.png\0\0");
	ofn.nFilterIndex=1;
	ofn.lpstrTitle=_T("Save As PNG");
	
	ofn.lpstrInitialDir=NULL;
	ofn.lpstrFile=fn;
	ofn.nMaxFile=MAX_PATH;
	ofn.Flags=OFN_PATHMUSTEXIST|OFN_HIDEREADONLY|OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt=_T("png");

	if(!GetSaveFileName(&ofn)) return;  /* canceled by user */

	pngdib=pngdib_d2p_init();
	pngdib_d2p_set_png_filename(pngdib,fn);
	pngdib_d2p_set_dib(pngdib,img->dib,0,NULL,0);
	pngdib_d2p_set_software_id(pngdib,_T("SMVIEW"));
	pngdib_d2p_set_gamma_label(pngdib, g->use_gamma, PNGDIB_DEFAULT_FILE_GAMMA);

	ret=pngdib_d2p_run(pngdib);

	if(ret) {
		StringCbPrintf(buf,sizeof(buf),_T("Cannot save file: %s (%d)"),pngdib_get_error_msg(pngdib),ret);
		MessageBox(hwnd,buf,MYNAME,MB_OK|MB_ICONWARNING);
	}

	pngdib_done(pngdib);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
   LPSTR lpCmdLine,int nCmdShow)
{
	MSG msg;
	WNDCLASS wc;
	HWND hwnd;
	int rv;

	g=calloc(sizeof(struct globals_struct),1);
	img=calloc(sizeof(struct img_struct),1);
	if(!g || !img) return 0;

	g->use_bkgd=1; g->use_custom_bg=1; g->use_gamma=1;
	g->image_bg = RGB(255,255,255);
	g->custom_bg = RGB(240,240,240);
	g->which_bg=0;
	g->hInst=hInstance;
	StringCbCopy(img->png_filename,MAX_PATH,_T(""));

	ZeroMemory(&wc,sizeof(WNDCLASS));
	wc.style = CS_DBLCLKS;
	wc.lpfnWndProc = (WNDPROC)WndProcMain;
	wc.hInstance = g->hInst;
	wc.hIcon = LoadIcon(g->hInst,_T("ICONMAIN"));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszMenuName =  _T("MENUMAIN");
	wc.lpszClassName = _T("SMVIEWCLASS");
	if(!RegisterClass(&wc)) return 0;

	hwnd = CreateWindow(_T("SMVIEWCLASS"),MYNAME,WS_VISIBLE|WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,
		NULL,NULL,g->hInst,NULL);
	if(!hwnd) return 0;

	DragAcceptFiles(hwnd,TRUE);

	while(1)
	{
		rv=GetMessage(&msg,NULL,0,0);
		if(!rv || rv== -1) break;
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	free(img);
	free(g);
	return (int)msg.wParam;
}

static LRESULT CALLBACK WndProcMain(HWND hwnd, UINT msg,
     WPARAM wParam, LPARAM lParam)
{
	WORD id;
	id=LOWORD(wParam);

	switch(msg) {

	case WM_ERASEBKGND:
		{
			HDC hdc;
			RECT rect;
			HBRUSH oldbr,br;
			int delbrush;

			delbrush=0;
			hdc=(HDC)wParam;
			if(!GetClientRect(hwnd,&rect)) return 0;
			if(g->which_bg) {
				br=CreateSolidBrush(g->image_bg);
				delbrush=1;
			}
			else {
				if(g->use_custom_bg) {
					br=CreateSolidBrush(g->custom_bg);
					delbrush=1;
				}
				else {
					br=GetSysColorBrush(COLOR_WINDOW);
				}
			}
			oldbr=(HBRUSH)SelectObject(hdc,br);
			FillRect(hdc,&rect,br);
			SelectObject(hdc,oldbr);
			if(delbrush) DeleteObject(br);
		}
		return 1;

	case WM_PAINT:
		{
			PAINTSTRUCT paintStruct;
			HDC hdc;

			hdc = BeginPaint(hwnd, &paintStruct);
			if(img->dib) {
				StretchDIBits(hdc,
					0,0,img->dib->biWidth,img->dib->biHeight,
					0,0,img->dib->biWidth,img->dib->biHeight,
					img->bits,(LPBITMAPINFO)img->dib,
					DIB_RGB_COLORS,SRCCOPY);
			}
			EndPaint(hwnd, &paintStruct);
			return 0;
		}

	case WM_DESTROY:
		if(img->dib) { pngdib_p2d_free_dib(NULL,img->dib); img->dib=NULL; }
		PostQuitMessage(0);
		return 0;

	case WM_DROPFILES:
		dropped_files(hwnd, (HDROP)wParam);
		return 0;

	case WM_INITMENU:
		CheckMenuItem((HMENU)wParam, ID_BG_BKGD , MF_BYCOMMAND|
			(g->use_bkgd?MF_CHECKED:MF_UNCHECKED) );
		CheckMenuItem((HMENU)wParam, ID_BG_CUSTOM , MF_BYCOMMAND|
			(g->use_custom_bg?MF_CHECKED:MF_UNCHECKED) );
		CheckMenuItem((HMENU)wParam, ID_GAMMACORRECT , MF_BYCOMMAND|
			(g->use_gamma?MF_CHECKED:MF_UNCHECKED) );
		EnableMenuItem((HMENU)wParam, ID_CLOSE, MF_BYCOMMAND|
			(img->dib?MF_ENABLED:MF_GRAYED) );
		EnableMenuItem((HMENU)wParam, ID_SAVEASPNG, MF_BYCOMMAND|
			(img->dib?MF_ENABLED:MF_GRAYED) );
		EnableMenuItem((HMENU)wParam, ID_SAVEASBMP, MF_BYCOMMAND|
			(img->dib?MF_ENABLED:MF_GRAYED) );
		return 0;

	case WM_COMMAND:
		switch(id) {
		case ID_EXIT:
			DestroyWindow(hwnd);
			return 0;
		case ID_OPEN:
			open_png_dialog(hwnd);
			return 0;
		case ID_SAVEASPNG:
			if(img->dib) save_as_png(hwnd);
			return 0;
		case ID_CLOSE:
			if(img->dib) { pngdib_p2d_free_dib(NULL,img->dib); img->dib=NULL; }
			StringCchCopy(img->png_filename,MAX_PATH,_T(""));
			g->which_bg=0;
			InvalidateRect(hwnd,NULL,TRUE);
			return 0;
		case ID_BG_BKGD:
			g->use_bkgd = !g->use_bkgd;
			open_png_by_name(hwnd,img->png_filename); /* reload image */
			return 0;
		case ID_BG_CUSTOM:
			g->use_custom_bg = !g->use_custom_bg;
			open_png_by_name(hwnd,img->png_filename);
			return 0;
		case ID_GAMMACORRECT:
			g->use_gamma = !g->use_gamma;
			open_png_by_name(hwnd,img->png_filename);
			return 0;
		case ID_SETBG:
			if(choose_color_dialog(hwnd,&g->custom_bg)) {
				open_png_by_name(hwnd,img->png_filename);
			}
			return 0;
		case ID_ABOUT:
			DialogBox(g->hInst,_T("ABOUTDLG"),hwnd,DlgProcAbout);
			return 0;
		}
		break;
	}
	return DefWindowProc(hwnd, msg, wParam, lParam);
}

static INT_PTR CALLBACK DlgProcAbout(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	WORD id;
	TCHAR buf[1000];
	id=LOWORD(wParam);

	switch (msg) {

	case WM_INITDIALOG: 
		StringCbPrintf(buf,sizeof(buf),_T("Demonstration PNG image viewer that uses the PNGDIB library.\r\n")
			_T("Version %s\r\nBy Jason Summers, 8/2000-%s\r\nPNGDIB version %s"),
			SMVIEW_VERS,SMVIEW_DATE,pngdib_get_version_string());
		SetDlgItemText(hwnd,IDC_TEXT1,buf);

		return 1;
	case WM_COMMAND:
		switch(id) {
		case IDOK:
		case IDCANCEL:
			EndDialog(hwnd, 0);
			return 1;
		}
	}
	return 0;
}
