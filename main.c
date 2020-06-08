#include<windows.h>
#include<windowsx.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>
#include"main.h"
#include"system.h"

PCANVAS canvas;
PMINEFIELD mf;
INFO info;
BESTTIME bt;
const INFO preset[TYPE_COUNT] = {
	{1,9,9,10},
	{1,16,16,40},
	{1,30,16,99},
	{2,9,9,15},
	{2,16,16,60},
	{2,30,16,149},
	{3,9,9,20},
	{3,16,16,80},
	{3,30,16,199},
	{4,9,9,25},
	{4,16,16,100},
	{4,30,16,249},
	{5,9,9,30},
	{5,16,16,120},
	{5,30,16,299},
	{6,9,9,35},
	{6,16,16,140},
	{6,30,16,349},
	{7,9,9,40},
	{7,16,16,160},
	{7,30,16,399}
};
char path[MAX_PATH];

LRESULT CALLBACK DlgBesttimeProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg){
        case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDOK:
				case IDCANCEL:
					EndDialog(hwnd,LOWORD(wParam));
			}
        	break;
        case WM_INITDIALOG:{
        	SHORT i;
        	for(i=0;i<TYPE_COUNT;++i)if(bt.record[i]>0)SetDlgItemInt(hwnd,BEST_TIME+i,bt.record[i],FALSE);
        	break;
        }
        default:return FALSE;
    }
    return TRUE;
}

LRESULT CALLBACK DlgCustomProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	switch(msg){
        case WM_COMMAND:
			switch(LOWORD(wParam)){
				case IDOK:{
					SHORT i,width,height,multi,max;
					width=GetDlgItemInt(hwnd,CUSTOM_WIDTH,NULL,FALSE);
					height=GetDlgItemInt(hwnd,CUSTOM_HEIGHT,NULL,FALSE);
					multi=GetDlgItemInt(hwnd,CUSTOM_MULTI,NULL,FALSE);
					info.mine=GetDlgItemInt(hwnd,CUSTOM_MINE,NULL,FALSE);
					if(width<WIDTH_MIN)width=WIDTH_MIN;else if(width>WIDTH_MAX)width=WIDTH_MAX;
					if(height<HEIGHT_MIN)height=HEIGHT_MIN;else if(height>HEIGHT_MAX)height=HEIGHT_MAX;
					if(multi==0)multi=1;else if(multi>7)multi=7;
					max=width*height-9;
					if(info.mine==0)info.mine=1;else if(info.mine>max)info.mine=max;
					info.width=width;
					info.height=height;
					info.multi=multi;
					InitCanvas(canvas,mf,&info);
					bt.type=TYPE_CUSTOM;
					HMENU hMenu=GetMenu(GetParent(hwnd));
					for(i=1;i<=7;++i)CheckMenuItem(hMenu,ID_MULTI+i,i==multi?MF_CHECKED:MF_UNCHECKED);
				}
				case IDCANCEL:
					EndDialog(hwnd,LOWORD(wParam));
			}
        	break;
        case WM_INITDIALOG:
			SetDlgItemInt(hwnd,CUSTOM_WIDTH,mf->width,FALSE);
			SetDlgItemInt(hwnd,CUSTOM_HEIGHT,mf->height,FALSE);
			SetDlgItemInt(hwnd,CUSTOM_MULTI,mf->multi,FALSE);
			SetDlgItemInt(hwnd,CUSTOM_MINE,mf->mine,FALSE);
			break;
        default:return FALSE;
    }
    return TRUE;
}

LRESULT CALLBACK WndProc(HWND hwnd,UINT msg,WPARAM wParam,LPARAM lParam) {
	static PAINTSTRUCT ps;
	static HMENU hMenu;
	static UCHAR multi;
	switch(msg) {
		case WM_PAINT:
			BeginPaint(hwnd,&ps);
			BitBlt(ps.hdc,0,0,canvas->rc.right,canvas->rc.bottom,canvas->hdc,0,0,SRCCOPY);
			EndPaint(hwnd,&ps);
			break;
		case WM_MOUSEMOVE:if(wParam&(MK_LBUTTON|MK_MBUTTON))DragCursor(canvas,mf,(POINT){GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)});break;
		case WM_LBUTTONDOWN:{
			POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
			if(PtInRect(&canvas->rcSprite,pt))canvas->clkstate=CLK_RESTART;
			if(mf->progress!=0&&PtInRect(&canvas->rcField,pt)){
				canvas->clkstate=(canvas->clkstate==CLK_PRECHORD?CLK_CHORD:CLK_REVEAL);
				UpdateSprite(canvas,SPRITE_REVEAL);
			}
			DragCursor(canvas,mf,pt);
			break;
		}
		case WM_LBUTTONUP:{
			if(mf->progress!=0)UpdateSprite(canvas,SPRITE_NORMAL);
			POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
			if(canvas->clkstate==CLK_RESTART&&PtInRect(&canvas->rcSprite,pt))if(mf->progress>=0)InitCanvas(canvas,mf,&info);
			if(mf->progress!=0&&PtInRect(&canvas->rcField,pt)){
				if(canvas->clkstate==CLK_CHORD)ChordField(canvas,mf,&bt);
				else RevealField(canvas,mf,(pt.x-BORDER_SIZE)/FIELD_SIZE,(pt.y-FIELD_TOPPAD)/FIELD_SIZE);
			}
			/* FALSE REVEAL */
			if(canvas->clkstate&CLK_LOSE)TerminateGame(canvas,mf,FALSE,&bt);
			/* FINAL REVEAL */
			else if(canvas->clkstate&CLK_WIN)TerminateGame(canvas,mf,TRUE,&bt);
			ReleaseMouse(canvas);
			break;
		}
		case WM_RBUTTONDOWN:{
			if(mf->progress==0)break;
			canvas->clkstate=CLK_PRECHORD;
			POINT pt={GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)};
			if(PtInRect(&canvas->rcField,pt))FlagField(canvas,mf,(pt.x-BORDER_SIZE)/FIELD_SIZE,(pt.y-FIELD_TOPPAD)/FIELD_SIZE);
			break;
		}
		case WM_RBUTTONUP:
			if(canvas->clkstate==CLK_CHORD){
				ChordField(canvas,mf,&bt);
				canvas->clkstate=CLK_REVEAL;
			} else canvas->clkstate=CLK_RELEASED;
			break;
		case WM_MBUTTONDOWN:
			if(mf->progress==0)break;
			canvas->clkstate=CLK_CHORD;
			DragCursor(canvas,mf,(POINT){GET_X_LPARAM(lParam),GET_Y_LPARAM(lParam)});
			UpdateSprite(canvas,SPRITE_REVEAL);
			break;
		case WM_MBUTTONUP:
			if(mf->progress==0)break;
			UpdateSprite(canvas,SPRITE_NORMAL);
			ChordField(canvas,mf,&bt);
			break;
		case WM_HOTKEY:
			switch(wParam){
				case HOT_RESTART:if(mf->progress>=0)InitCanvas(canvas,mf,&info);break;
				case HOT_CUSTOM:DialogBox(NULL,MAKEINTRESOURCE(DLG_CUSTOM),hwnd,DlgCustomProc);break;
				case HOT_BESTTIME:DialogBox(NULL,MAKEINTRESOURCE(DLG_BESTTIME),hwnd,DlgBesttimeProc);break;
				default:return DefWindowProc(hwnd,msg,wParam,lParam);
			}
			break;
		case WM_TIMER:if(wParam==MAIN_TIMER)IncrementTimer(canvas);break;
		case WM_COMMAND:
			switch(LOWORD(wParam)){
				case ID_BEGINNER:
				case ID_INTERMEDIATE:
				case ID_EXPERT:
					bt.type=LOWORD(wParam)+multi*TYPE_MULTI-ID_PRESET-TYPE_MULTI;
					info=preset[bt.type];
					InitCanvas(canvas,mf,&info);
					break;
				case ID_SINGLE:
				case ID_DOUBLE:
				case ID_TRIPLE:
				case ID_QUADRUPLE:
				case ID_QUINTUPLE:
				case ID_SEXTUPLE:
				case ID_SEPTUPLE:{
					int i;
					multi=LOWORD(wParam)-ID_MULTI;
					for(i=1;i<=7;++i)CheckMenuItem(hMenu,ID_MULTI+i,i==multi?MF_CHECKED:MF_UNCHECKED);
					break;
				}
				case ID_CUSTOM:DialogBox(NULL,MAKEINTRESOURCE(DLG_CUSTOM),hwnd,DlgCustomProc);break;
				case ID_RESTART:if(mf->progress>=0)InitCanvas(canvas,mf,&info);break;
				case ID_BESTTIME:DialogBox(NULL,MAKEINTRESOURCE(DLG_BESTTIME),hwnd,DlgBesttimeProc);break;
				case ID_EXIT:PostQuitMessage(0);break;
				default:return DefWindowProc(hwnd,msg,wParam,lParam);
			}
			break;
		case WM_CREATE:
			hMenu=GetMenu(hwnd);
			RegisterHotKey(hwnd,HOT_RESTART,MOD_SHIFT,'R');
			RegisterHotKey(hwnd,HOT_CUSTOM,MOD_SHIFT,'C');
			RegisterHotKey(hwnd,HOT_BESTTIME,MOD_SHIFT,'B');
			mf=CreateMinefield();
			canvas=CreateCanvas(hwnd);
			info=preset[TYPE_DEFAULT];
			multi=LoadFile(mf,canvas,&info,&bt,path);
			if(!multi){multi=1;InitCanvas(canvas,mf,&info);}
			break;
		case WM_DESTROY:
			KillTimer(hwnd,MAIN_TIMER);
			SaveFile(mf,canvas->time,&bt,path);
			DestroyMinefield(mf);
			DestroyCanvas(canvas);
			PostQuitMessage(0);
			break;
		default:return DefWindowProc(hwnd,msg,wParam,lParam);
	}
	return 0;
}

int WINAPI WinMain(HINSTANCE hInst,HINSTANCE hPrevInst,LPSTR lpCmdLine,int nCmdShow) {
	srand(time(NULL));
	
	WNDCLASS wc;
	HWND hwnd;
	MSG msg;

	memset(&wc,0,sizeof(wc));
	wc.lpfnWndProc	 = WndProc;
	wc.hInstance	 = hInst;
	wc.hIcon		 = LoadIcon(hInst,"A");
	wc.hCursor		 = LoadCursor(NULL,IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
	wc.lpszMenuName	 = MAKEINTRESOURCE(MAIN_MENU);
	wc.lpszClassName = "WindowClass";
	if(!RegisterClass(&wc)){MessageBox(NULL,"Window Registration Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);return 0;}

	hwnd=CreateWindow("WindowClass","Multi Mine Sweeper",WINDOW_STYLE,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,CW_USEDEFAULT,NULL,NULL,hInst,NULL);
	if(hwnd==NULL){MessageBox(NULL,"Window Creation Failed!","Error!",MB_ICONEXCLAMATION|MB_OK);return 0;}

	while(GetMessage(&msg,NULL,0,0)>0){
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam;
}
