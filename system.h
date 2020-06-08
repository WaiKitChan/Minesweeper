/* BASIC */
typedef struct info {
	CHAR multi;
	CHAR width;
	CHAR height;
	SHORT mine;
} INFO,*PINFO;
typedef struct besttime {
	SHORT type;
	SHORT record[TYPE_COUNT];
} BESTTIME,*PBESTTIME;
typedef struct field {
	UCHAR state;
	UCHAR count;
} FIELD,*PFIELD;
typedef struct minefield {
	CHAR multi;
	CHAR width;
	CHAR height;
	SHORT mine;
	SHORT remain;
	SHORT progress;
	FIELD field[FIELD_MAX];
} MINEFIELD,*PMINEFIELD;
PMINEFIELD CreateMinefield() {return(PMINEFIELD)malloc(sizeof(MINEFIELD));}
void DestroyMinefield(PMINEFIELD mf) {free(mf);}

/* CANVAS */
typedef struct canvas {
	SHORT time;
	CHAR clkstate;
	POINT cursor;
	HWND hwnd;
	RECT rc;
	RECT rcSprite;
	RECT rcCounter;
	RECT rcTimer;
	RECT rcField;
	HDC hdc;
	HDC sprite;
	HDC digit;
	HDC number;
	HDC mine;
	HDC shade;
	HBRUSH hbrBody;
	HBRUSH hbrDark;
	HBRUSH hbrBright;
	HGDIOBJ oldDC;
	HGDIOBJ oldSprite;
	HGDIOBJ oldDigit;
	HGDIOBJ oldNumber;
	HGDIOBJ oldMine;
	HGDIOBJ oldShade;
} CANVAS,*PCANVAS;
PCANVAS CreateCanvas(HWND hwnd) {
	PCANVAS canvas=(PCANVAS)malloc(sizeof(CANVAS));
	canvas->hwnd=hwnd;
	/* FIX LEFT AND TOP */
	canvas->rc=(RECT){0,0,GLOBAL_WIDTH,GLOBAL_HEIGHT};
	canvas->rcField=(RECT){BORDER_SIZE,FIELD_TOPPAD,0,0};
	/* FIX ALL FOUR SIDES */
	canvas->rcCounter=(RECT){BORDER_SIZE+HEADER_SIDEPAD,BORDER_SIZE+DIGIT_TOPPAD,BORDER_SIZE+HEADER_SIDEPAD+DIGIT_WIDTH*4,BORDER_SIZE+DIGIT_TOPPAD+DIGIT_HEIGHT};
	/* FIX TOP AND BOTTOM */
	canvas->rcSprite=(RECT){0,BORDER_SIZE+SPRITE_TOPPAD,SPRITE_SIZE,BORDER_SIZE+SPRITE_TOPPAD+SPRITE_SIZE};
	canvas->rcTimer=(RECT){0,BORDER_SIZE+DIGIT_TOPPAD,DIGIT_WIDTH*4,BORDER_SIZE+DIGIT_TOPPAD+DIGIT_HEIGHT};
	HDC hdc=GetDC(hwnd);
	canvas->hdc=CreateCompatibleDC(hdc);
	canvas->sprite=CreateCompatibleDC(hdc);
	canvas->digit=CreateCompatibleDC(hdc);
	canvas->number=CreateCompatibleDC(hdc);
	canvas->mine=CreateCompatibleDC(hdc);
	canvas->shade=CreateCompatibleDC(hdc);
	HINSTANCE hInst=GetModuleHandle(NULL);
	HBITMAP hbm=CreateCompatibleBitmap(hdc,GLOBAL_WIDTH,GLOBAL_HEIGHT);
	canvas->oldDC=SelectObject(canvas->hdc,hbm);
	hbm=LoadBitmap(hInst,MAKEINTRESOURCE(BM_SPRITE));
	canvas->oldSprite=SelectObject(canvas->sprite,hbm);
	hbm=LoadBitmap(hInst,MAKEINTRESOURCE(BM_DIGIT));
	canvas->oldDigit=SelectObject(canvas->digit,hbm);
	hbm=LoadBitmap(hInst,MAKEINTRESOURCE(BM_NUMBER));
	canvas->oldNumber=SelectObject(canvas->number,hbm);
	hbm=LoadBitmap(hInst,MAKEINTRESOURCE(BM_MINE));
	canvas->oldMine=SelectObject(canvas->mine,hbm);
	canvas->hbrBody=CreateSolidBrush(COLOR_BODY);
	canvas->hbrDark=CreateSolidBrush(COLOR_DARK);
	canvas->hbrBright=CreateSolidBrush(COLOR_BRIGHT);
	hbm=CreateCompatibleBitmap(hdc,SHADE_SIZE*2,SHADE_SIZE);
	canvas->oldShade=SelectObject(canvas->shade,hbm);
	RECT rc={0,0,SHADE_SIZE*2,SHADE_SIZE};
	FillRect(canvas->shade,&rc,canvas->hbrBody);
	SetPixel(canvas->shade,SHADE_INNER,0,COLOR_DARK);
	SetPixel(canvas->shade,SHADE_INNER+1,1,COLOR_BRIGHT);
	SetPixel(canvas->shade,SHADE_OUTER,0,COLOR_BRIGHT);
	SetPixel(canvas->shade,SHADE_OUTER+1,1,COLOR_DARK);
	ReleaseDC(hwnd,hdc);
	return canvas;
}
void DestroyCanvas(PCANVAS canvas) {
	DeleteObject(SelectObject(canvas->hdc,canvas->oldDC));
	DeleteObject(SelectObject(canvas->sprite,canvas->oldSprite));
	DeleteObject(SelectObject(canvas->digit,canvas->oldDigit));
	DeleteObject(SelectObject(canvas->number,canvas->oldNumber));
	DeleteObject(SelectObject(canvas->mine,canvas->oldMine));
	DeleteObject(SelectObject(canvas->shade,canvas->oldShade));
	DeleteDC(canvas->hdc);
	DeleteDC(canvas->sprite);
	DeleteDC(canvas->digit);
	DeleteDC(canvas->number);
	DeleteDC(canvas->mine);
	DeleteDC(canvas->shade);
	DeleteObject(canvas->hbrBody);
	DeleteObject(canvas->hbrDark);
	DeleteObject(canvas->hbrBright);
	free(canvas);
}
void UpdateCounter(PCANVAS canvas,SHORT remain) {
	if(remain<0){
		BitBlt(canvas->hdc,BORDER_SIZE+HEADER_SIDEPAD,BORDER_SIZE+DIGIT_TOPPAD,DIGIT_WIDTH,DIGIT_HEIGHT,canvas->digit,MINUS_OFFSET,0,SRCCOPY);
		remain=-remain;
	} else BitBlt(canvas->hdc,BORDER_SIZE+HEADER_SIDEPAD,BORDER_SIZE+DIGIT_TOPPAD,DIGIT_WIDTH,DIGIT_HEIGHT,canvas->digit,DIGIT_WIDTH*(remain/1000),0,SRCCOPY);
	BitBlt(canvas->hdc,BORDER_SIZE+HEADER_SIDEPAD+DIGIT_WIDTH,BORDER_SIZE+DIGIT_TOPPAD,DIGIT_WIDTH,DIGIT_HEIGHT,canvas->digit,DIGIT_WIDTH*(remain%1000/100),0,SRCCOPY);
	BitBlt(canvas->hdc,BORDER_SIZE+HEADER_SIDEPAD+DIGIT_WIDTH*2,BORDER_SIZE+DIGIT_TOPPAD,DIGIT_WIDTH,DIGIT_HEIGHT,canvas->digit,DIGIT_WIDTH*(remain%100/10),0,SRCCOPY);
	BitBlt(canvas->hdc,BORDER_SIZE+HEADER_SIDEPAD+DIGIT_WIDTH*3,BORDER_SIZE+DIGIT_TOPPAD,DIGIT_WIDTH,DIGIT_HEIGHT,canvas->digit,DIGIT_WIDTH*(remain%10),0,SRCCOPY);
	InvalidateRect(canvas->hwnd,&canvas->rcCounter,FALSE);
}
void IncrementTimer(PCANVAS canvas) {
	if(++canvas->time==10000){
		KillTimer(canvas->hwnd,MAIN_TIMER);
		return;
	}
	SHORT l=canvas->rcTimer.right-DIGIT_WIDTH,t=canvas->rcTimer.top,d=canvas->time%10,mod=10,div=1;
	BitBlt(canvas->hdc,l,t,DIGIT_WIDTH,DIGIT_HEIGHT,canvas->digit,DIGIT_WIDTH*d,0,SRCCOPY);
	while(d==0){
		l-=DIGIT_WIDTH;
		mod*=10;
		div*=10;
		d=canvas->time%mod/div;
		BitBlt(canvas->hdc,l,t,DIGIT_WIDTH,DIGIT_HEIGHT,canvas->digit,DIGIT_WIDTH*d,0,SRCCOPY);
	}
	RECT rc={l,t,canvas->rcTimer.right,canvas->rcTimer.bottom};
	InvalidateRect(canvas->hwnd,&rc,FALSE);
}
void RepaintField(PCANVAS canvas,PMINEFIELD mf) {
	SHORT i,j,pos=0,mem;
	for(j=0;j<mf->height;++j)for(i=0;i<mf->width;++i){
		if(ISTRIGGERED(mf->field[pos].state))BitBlt(canvas->hdc,BORDER_SIZE+FIELD_SIZE*i,FIELD_TOPPAD+FIELD_SIZE*j,FIELD_SIZE,FIELD_SIZE,canvas->mine,FIELD_SIZE*GETMINE(mf->field[pos].state),FIELD_SIZE*2,SRCCOPY);
		else if(ISREVEALED(mf->field[pos].state)){
			mem=GETMINE(mf->field[pos].state);
			if(mem)BitBlt(canvas->hdc,BORDER_SIZE+FIELD_SIZE*i,FIELD_TOPPAD+FIELD_SIZE*j,FIELD_SIZE,FIELD_SIZE,canvas->mine,FIELD_SIZE*mem,FIELD_SIZE,SRCCOPY);
			else {
				mem=mf->field[pos].count;
				if(mem<10)BitBlt(canvas->hdc,BORDER_SIZE+FIELD_SIZE*i,FIELD_TOPPAD+FIELD_SIZE*j,FIELD_SIZE,FIELD_SIZE,canvas->number,FIELD_SIZE*mem,0,SRCCOPY);
				else {
					BitBlt(canvas->hdc,BORDER_SIZE+FIELD_SIZE*i,FIELD_TOPPAD+FIELD_SIZE*j,FIELD_HSIZE,FIELD_SIZE,canvas->number,FIELD_SIZE*(mem%100/10),FIELD_SIZE,SRCCOPY);
					BitBlt(canvas->hdc,BORDER_SIZE+FIELD_HSIZE+FIELD_SIZE*i,FIELD_TOPPAD+FIELD_SIZE*j,FIELD_HSIZE,FIELD_SIZE,canvas->number,FIELD_SIZE*(mem%10)+FIELD_HSIZE,FIELD_SIZE,SRCCOPY);
				}
			}
		} else BitBlt(canvas->hdc,BORDER_SIZE+FIELD_SIZE*i,FIELD_TOPPAD+FIELD_SIZE*j,FIELD_SIZE,FIELD_SIZE,canvas->mine,FIELD_SIZE*GETSTATE(mf->field[pos].state),0,SRCCOPY);
		++pos;
	}
	InvalidateRect(canvas->hwnd,&canvas->rcField,FALSE);
}
void UpdateSprite(PCANVAS canvas,SHORT state) {
	BitBlt(canvas->hdc,canvas->rcSprite.left,canvas->rcSprite.top,SPRITE_SIZE,SPRITE_SIZE,canvas->sprite,SPRITE_SIZE*state,0,SRCCOPY);
	InvalidateRect(canvas->hwnd,&canvas->rcSprite,FALSE);
}
void ReleaseMouse(PCANVAS canvas) {
	canvas->clkstate=CLK_RELEASED;
	canvas->cursor.x=0;
	canvas->cursor.y=0;
}
void DragCursor(PCANVAS canvas,PMINEFIELD mf,POINT pt) {
	BOOL i,j;
	switch(canvas->clkstate){
		case CLK_RESTART:
			i=PtInRect(&canvas->rcSprite,canvas->cursor);
			j=PtInRect(&canvas->rcSprite,pt);
			if(i^j){
				BitBlt(canvas->hdc,canvas->rcSprite.left,canvas->rcSprite.top,SPRITE_SIZE,SPRITE_SIZE,canvas->sprite,SPRITE_SIZE*j,0,SRCCOPY);
				InvalidateRect(canvas->hwnd,&canvas->rcSprite,FALSE);
			}
			if(mf->progress==0)break;
		case CLK_REVEAL:
			i=PtInRect(&canvas->rcField,canvas->cursor);
			j=PtInRect(&canvas->rcField,pt);
			if(i|j){
				BOOL k=i;
				SHORT x,y,n;
				x=(canvas->cursor.x-BORDER_SIZE)/FIELD_SIZE;
				y=(canvas->cursor.y-FIELD_TOPPAD)/FIELD_SIZE;
				for(n=0;n<2;++n,k=j,x=(pt.x-BORDER_SIZE)/FIELD_SIZE,y=(pt.y-FIELD_TOPPAD)/FIELD_SIZE){
					if(k&&GETSTATE(mf->field[x+y*mf->width].state)==0){
						RECT rc={BORDER_SIZE+FIELD_SIZE*x,FIELD_TOPPAD+FIELD_SIZE*y,0,0};
						rc.right=rc.left+FIELD_SIZE;
						rc.bottom=rc.top+FIELD_SIZE;
						BitBlt(canvas->hdc,rc.left,rc.top,FIELD_SIZE,FIELD_SIZE,canvas->mine,0,FIELD_SIZE*n,SRCCOPY);
						InvalidateRect(canvas->hwnd,&rc,FALSE);
					}
				}
			}
			break;
		case CLK_CHORD:
			i=PtInRect(&canvas->rcField,canvas->cursor);
			j=PtInRect(&canvas->rcField,pt);
			if(i|j){
				BOOL k=i;
				SHORT x,y,n,p,q;
				x=(canvas->cursor.x-BORDER_SIZE)/FIELD_SIZE-1;
				y=(canvas->cursor.y-FIELD_TOPPAD)/FIELD_SIZE-1;
				for(n=0;n<2;++n,k=j,x=(pt.x-BORDER_SIZE)/FIELD_SIZE-1,y=(pt.y-FIELD_TOPPAD)/FIELD_SIZE-1)if(k){
					RECT rc={BORDER_SIZE+FIELD_SIZE*x,FIELD_TOPPAD+FIELD_SIZE*y,0,0};
					rc.right=rc.left+FIELD_SIZE*3;
					rc.bottom=rc.top+FIELD_SIZE*3;
					for(p=0;p<3;++p)for(q=0;q<3;++q)if(x+p>=0&&x+p<mf->width&&y+q>=0&&y+q<mf->height)
						if(GETSTATE(mf->field[x+p+(y+q)*mf->width].state)==0)BitBlt(canvas->hdc,rc.left+FIELD_SIZE*p,rc.top+FIELD_SIZE*q,FIELD_SIZE,FIELD_SIZE,canvas->mine,0,FIELD_SIZE*n,SRCCOPY);
					InvalidateRect(canvas->hwnd,&rc,FALSE);
				}
			}
	}
	canvas->cursor=pt;
}

/* INITIATION */
void InitCanvas(PCANVAS canvas,PMINEFIELD mf,const INFO *info) {
	mf->multi=info->multi;
	mf->width=info->width;
	mf->height=info->height;
	mf->mine=info->mine;
	mf->remain=info->mine;
	mf->progress=-info->mine;
	memset(&mf->field,0,sizeof(FIELD)*FIELD_MAX);
	canvas->time=0;
	canvas->clkstate=0;
	canvas->cursor.x=0;
	canvas->cursor.y=0;
	FillRect(canvas->hdc,&canvas->rc,canvas->hbrBody);
	canvas->rcField.right=BORDER_SIZE+FIELD_SIZE*info->width;
	canvas->rcField.bottom=FIELD_TOPPAD+FIELD_SIZE*info->height;
	canvas->rc.right=canvas->rcField.right+BORDER_SIZE;
	canvas->rc.bottom=canvas->rcField.bottom+BORDER_SIZE;
	canvas->rcTimer.right=canvas->rcField.right-HEADER_SIDEPAD;
	canvas->rcTimer.left=canvas->rcTimer.right-DIGIT_WIDTH*4;
	canvas->rcSprite.left=(canvas->rcField.left+canvas->rcField.right-SPRITE_SIZE)/2;
	canvas->rcSprite.right=canvas->rcSprite.left+SPRITE_SIZE;
	KillTimer(canvas->hwnd,MAIN_TIMER);
	/* RESIZE WINDOW */
	RECT rc;
	GetWindowRect(canvas->hwnd,&rc);
	const SHORT l=rc.left,t=rc.top;
	rc.right=l+canvas->rc.right;
	rc.bottom=t+canvas->rc.bottom;
	AdjustWindowRect(&rc,WINDOW_STYLE,TRUE);
	MoveWindow(canvas->hwnd,l,t,rc.right-rc.left,rc.bottom-rc.top,FALSE);
	InvalidateRect(canvas->hwnd,NULL,FALSE);
	/* PAINT BORDER */
	RECT rcshade=canvas->rc;
	rc=(RECT){0,0,SHADE_SIZE,rcshade.bottom};
	FillRect(canvas->hdc,&rc,canvas->hbrBright);
	rc=(RECT){0,0,rcshade.right,SHADE_SIZE};
	FillRect(canvas->hdc,&rc,canvas->hbrBright);
	rc=(RECT){0,rcshade.bottom-SHADE_SIZE,rcshade.right,rcshade.bottom};
	FillRect(canvas->hdc,&rc,canvas->hbrDark);
	rc=(RECT){rcshade.right-SHADE_SIZE,0,rcshade.right,rcshade.bottom};
	FillRect(canvas->hdc,&rc,canvas->hbrDark);
	BitBlt(canvas->hdc,0,rcshade.bottom-SHADE_SIZE,SHADE_SIZE,SHADE_SIZE,canvas->shade,SHADE_OUTER,0,SRCCOPY);
	BitBlt(canvas->hdc,rcshade.right-SHADE_SIZE,0,SHADE_SIZE,SHADE_SIZE,canvas->shade,SHADE_OUTER,0,SRCCOPY);
	SHORT i,j;
	rcshade=(RECT){BORDER_SIZE,BORDER_SIZE,rcshade.right-BORDER_SIZE,BORDER_SIZE+HEADER_HEIGHT};
	for(i=0;++i<=2;rcshade=canvas->rcField){
		rc=(RECT){rcshade.left-SHADE_SIZE,rcshade.top-SHADE_SIZE,rcshade.left,rcshade.bottom};
		FillRect(canvas->hdc,&rc,canvas->hbrDark);
		rc=(RECT){rcshade.left,rcshade.top-SHADE_SIZE,rcshade.right,rcshade.top};
		FillRect(canvas->hdc,&rc,canvas->hbrDark);
		rc=(RECT){rcshade.left,rcshade.bottom,rcshade.right+SHADE_SIZE,rcshade.bottom+SHADE_SIZE};
		FillRect(canvas->hdc,&rc,canvas->hbrBright);
		rc=(RECT){rcshade.right,rcshade.top,rcshade.right+SHADE_SIZE,rcshade.bottom};
		FillRect(canvas->hdc,&rc,canvas->hbrBright);
		BitBlt(canvas->hdc,rcshade.left-SHADE_SIZE,rcshade.bottom,SHADE_SIZE,SHADE_SIZE,canvas->shade,SHADE_INNER,0,SRCCOPY);
		BitBlt(canvas->hdc,rcshade.right,rcshade.top-SHADE_SIZE,SHADE_SIZE,SHADE_SIZE,canvas->shade,SHADE_INNER,0,SRCCOPY);
	}
	/* PAINT CONTENT */
	BitBlt(canvas->hdc,BORDER_SIZE+HEADER_SIDEPAD,BORDER_SIZE+DIGIT_TOPPAD,DIGIT_WIDTH,DIGIT_HEIGHT,canvas->digit,DIGIT_WIDTH*(info->mine/1000),0,SRCCOPY);
	BitBlt(canvas->hdc,BORDER_SIZE+HEADER_SIDEPAD+DIGIT_WIDTH,BORDER_SIZE+DIGIT_TOPPAD,DIGIT_WIDTH,DIGIT_HEIGHT,canvas->digit,DIGIT_WIDTH*(info->mine%1000/100),0,SRCCOPY);
	BitBlt(canvas->hdc,BORDER_SIZE+HEADER_SIDEPAD+DIGIT_WIDTH*2,BORDER_SIZE+DIGIT_TOPPAD,DIGIT_WIDTH,DIGIT_HEIGHT,canvas->digit,DIGIT_WIDTH*(info->mine%100/10),0,SRCCOPY);
	BitBlt(canvas->hdc,BORDER_SIZE+HEADER_SIDEPAD+DIGIT_WIDTH*3,BORDER_SIZE+DIGIT_TOPPAD,DIGIT_WIDTH,DIGIT_HEIGHT,canvas->digit,DIGIT_WIDTH*(info->mine%10),0,SRCCOPY);
	for(i=0;i<4;++i)BitBlt(canvas->hdc,canvas->rcTimer.left+DIGIT_WIDTH*i,canvas->rcTimer.top,DIGIT_WIDTH,DIGIT_HEIGHT,canvas->digit,0,0,SRCCOPY);
	BitBlt(canvas->hdc,canvas->rcSprite.left,canvas->rcSprite.top,SPRITE_SIZE,SPRITE_SIZE,canvas->sprite,0,0,SRCCOPY);
	for(i=0;i<info->width;++i)for(j=0;j<info->height;++j)BitBlt(canvas->hdc,BORDER_SIZE+FIELD_SIZE*i,FIELD_TOPPAD+FIELD_SIZE*j,FIELD_SIZE,FIELD_SIZE,canvas->mine,0,0,SRCCOPY);
}
void InitMinefield(PCANVAS canvas,PMINEFIELD mf,const SHORT x,const SHORT y) {
	SHORT i,j,k,n;
	const SHORT wid=mf->width,hei=mf->height,widx=wid+2,size=wid*hei,sizex=(wid+2)*(hei+2);
	SHORT buf[size];
	LONG pos,len=size;
	for(i=0;i<size;++i)buf[i]=i;
	/* CLEAR NEIGHBOURHOOD */
	for(i=x-1;i<=x+1;++i)for(j=y-1;j<=y+1;++j)if(i>=0&&i<wid&&j>=0&&j<hei){
		buf[i+j*wid]=-1;
		--len;
	}
	i=0,j=size-1;
	while(i<len){
		while(buf[i]>=0)++i;
		while(buf[j]<0)--j;
		buf[i++]=buf[j--];
	}
	/* LAY MINES */
	while(mf->progress<0){
		pos=rand()%len;
		n=rand()%mf->multi+1;
		if(n+mf->progress>0)n=-mf->progress;
		mf->progress+=n;
		mf->field[buf[pos]].state|=n;
		buf[pos]=buf[--len];
	}
	/* COUNT MINES */
	UCHAR cnt[sizex];
	memset(cnt,0,sizeof(UCHAR)*sizex);
	for(i=0;i<wid;++i)for(j=0;j<hei;++j)for(k=0;k<3;++k)for(n=0;n<3;++n)cnt[i+k+(j+n)*widx]+=GETMINE(mf->field[i+j*wid].state);
	for(i=0;i<wid;++i)for(j=0;j<hei;++j)mf->field[i+j*wid].count=cnt[i+1+(j+1)*widx];
	/* COUNT GOAL */
	for(i=0;i<size;++i)if(GETMINE(mf->field[i].state)==0)++mf->progress;
	/* SET TIMER */
	SetTimer(canvas->hwnd,MAIN_TIMER,1000,NULL);
	IncrementTimer(canvas);
}

/* TERMINATION */
void TerminateGame(PCANVAS canvas,PMINEFIELD mf,BOOL win,PBESTTIME bt){
	mf->progress=0;
	UpdateSprite(canvas,SPRITE_LOSE+win);
	SHORT i;
	const SHORT size=mf->width*mf->height;
	if(win){
		for(i=0;i<size;++i)if(ISUNREVEALED(mf->field[i].state))SETFLAG(mf->field[i].state,GETMINE(mf->field[i].state));
		for(i=0;i<4;++i)BitBlt(canvas->hdc,canvas->rcCounter.left+DIGIT_WIDTH*i,canvas->rcCounter.top,DIGIT_WIDTH,DIGIT_HEIGHT,canvas->digit,0,0,SRCCOPY);
		InvalidateRect(canvas->hwnd,&canvas->rcCounter,FALSE);
	} else {
		for(i=0;i<size;++i){
			if(GETMINE(mf->field[i].state)){
				if(!GETFLAG(mf->field[i].state))SETREVEALED(mf->field[i].state);
			} else if(GETFLAG(mf->field[i].state))SETTRIGGERED(mf->field[i].state);
		}
	}
	KillTimer(canvas->hwnd,MAIN_TIMER);
	RepaintField(canvas,mf);
	/* REGISTER BEST TIME */
	if(win&&bt->type>=0){
		
		if(bt->record[bt->type]==0)bt->record[bt->type]=canvas->time;
		else if(canvas->time<bt->record[bt->type])bt->record[bt->type]=canvas->time;
	}
}

/* INTERATION */
void FlagField(PCANVAS canvas,PMINEFIELD mf,const SHORT x,const SHORT y) {
	const SHORT pos=x+y*mf->width;
	if(ISREVEALED(mf->field[pos].state))return;
	mf->field[pos].state+=FLAG;
	if(GETSTATE(mf->field[pos].state)>mf->multi){
		CLEARFLAG(mf->field[pos].state);
		mf->remain+=mf->multi;
	} else --mf->remain;
	RECT rc={BORDER_SIZE+FIELD_SIZE*x,FIELD_TOPPAD+FIELD_SIZE*y,0,0};
	rc.right=rc.left+FIELD_SIZE;
	rc.bottom=rc.top+FIELD_SIZE;
	BitBlt(canvas->hdc,rc.left,rc.top,FIELD_SIZE,FIELD_SIZE,canvas->mine,FIELD_SIZE*GETSTATE(mf->field[pos].state),0,SRCCOPY);
	InvalidateRect(canvas->hwnd,&rc,FALSE);
	UpdateCounter(canvas,mf->remain);
}
void RevealField(PCANVAS canvas,PMINEFIELD mf,const SHORT x,const SHORT y) {
	if(x<0||x>=mf->width||y<0||y>=mf->height)return;
	const SHORT pos=x+y*mf->width;
	if(ISFLAGGED(mf->field[pos].state))return;
	SETREVEALED(mf->field[pos].state);
	/* LAY MINES UPON FIRST REVEAL */
	if(mf->progress<0)InitMinefield(canvas,mf,x,y);
	/* PAINT */
	BOOL trig=FALSE;
	SHORT mine=GETMINE(mf->field[pos].state),count=mf->field[pos].count;
	if(mine){
		BitBlt(canvas->hdc,BORDER_SIZE+FIELD_SIZE*x,FIELD_TOPPAD+FIELD_SIZE*y,FIELD_SIZE,FIELD_SIZE,canvas->mine,FIELD_SIZE*mine,FIELD_SIZE*2,SRCCOPY);
		SETTRIGGERED(mf->field[pos].state);
		canvas->clkstate|=CLK_LOSE;
	} else {
		RECT rc={BORDER_SIZE+FIELD_SIZE*x,FIELD_TOPPAD+FIELD_SIZE*y,0,0};
		rc.right=rc.left+FIELD_SIZE;
		rc.bottom=rc.top+FIELD_SIZE;{
			if(count<10)BitBlt(canvas->hdc,rc.left,rc.top,FIELD_SIZE,FIELD_SIZE,canvas->number,FIELD_SIZE*count,0,SRCCOPY);
			else {
				BitBlt(canvas->hdc,rc.left,rc.top,FIELD_HSIZE,FIELD_SIZE,canvas->number,FIELD_SIZE*(count%100/10),FIELD_SIZE,SRCCOPY);
				BitBlt(canvas->hdc,rc.left+FIELD_HSIZE,rc.top,FIELD_HSIZE,FIELD_SIZE,canvas->number,FIELD_SIZE*(count%10)+FIELD_HSIZE,FIELD_SIZE,SRCCOPY);
			}
		}
		InvalidateRect(canvas->hwnd,&rc,FALSE);
	}
	/* RECURSION */
	SHORT i,j;
	if(count==0)for(i=-1;i<=1;++i)for(j=-1;j<=1;++j)RevealField(canvas,mf,x+i,y+j);
	/* VICTORY CONDITION */
	if(--mf->progress==0)canvas->clkstate|=CLK_WIN;
}
void ChordField(PCANVAS canvas,PMINEFIELD mf,PBESTTIME bt) {
	if(!PtInRect(&canvas->rcField,canvas->cursor))return;
	SHORT i,j,k,n,x=(canvas->cursor.x-BORDER_SIZE)/FIELD_SIZE-1,y=(canvas->cursor.y-FIELD_TOPPAD)/FIELD_SIZE-1;
	SHORT pos=x+1+(y+1)*mf->width,flag=0;
	if(ISREVEALED(mf->field[pos].state)){
		for(i=0;i<3;++i)for(j=0;j<3;j++)if(x+i>=0&&x+i<mf->width&&y+j>=0&&y+j<mf->height)flag+=GETFLAG(mf->field[x+i+(y+j)*mf->width].state);
		if(flag==mf->field[pos].count){
			for(i=0;i<3;++i)for(j=0;j<3;++j)RevealField(canvas,mf,x+i,y+j);
			/* FALSE CHORDING */
			if(canvas->clkstate&CLK_LOSE)TerminateGame(canvas,mf,FALSE,bt);
			/* FINAL CHORDING */
			else if(canvas->clkstate&CLK_WIN)TerminateGame(canvas,mf,TRUE,bt);
			ReleaseMouse(canvas);
			return;
		}
	}
	RECT rc={BORDER_SIZE+FIELD_SIZE*x,FIELD_TOPPAD+FIELD_SIZE*y,0,0};
	rc.right=rc.left+FIELD_SIZE*3;
	rc.bottom=rc.top+FIELD_SIZE*3;
	for(i=0;i<3;++i)for(j=0;j<3;++j)if(x+i>=0&&x+i<mf->width&&y+j>=0&&y+j<mf->height)
		if(GETSTATE(mf->field[x+i+(y+j)*mf->width].state)==0)BitBlt(canvas->hdc,rc.left+FIELD_SIZE*i,rc.top+FIELD_SIZE*j,FIELD_SIZE,FIELD_SIZE,canvas->mine,0,0,SRCCOPY);
	ReleaseMouse(canvas);
	InvalidateRect(canvas->hwnd,&rc,FALSE);
}

/* SAVE AND LOAD */
typedef struct savedata {
	CHAR h;
	CHAR m;
	CHAR s;
	CHAR exist;
	BESTTIME bt;
	SHORT time;
	MINEFIELD mf;
} SAVEDATA,*PSAVEDATA;
BOOL LoadFile(PMINEFIELD mf,PCANVAS canvas,PINFO info,PBESTTIME bt,char *path) {
	LONG i;
	/* GET PATH TO APPDATA */
	char *env;
	env=getenv("APPDATA");
	/* SAVE LOCALLY UPON FAILURE */
	if(env==NULL)strcpy(path,"multmine.sav");
	else {
		strcpy(path,env);
		strcat(path,"\\MineSweeper");
		CreateDirectory(path,NULL);
		strcat(path,"\\multmine.sav");
	}
	FILE *f=fopen(path,"rb");
	if(f!=NULL){
		if(fgetc(f)=='M'&&fgetc(f)=='M'&&fgetc(f)=='S'){
			fseek(f,0,SEEK_SET);
			char data[sizeof(SAVEDATA)];
			for(i=0;i<sizeof(SAVEDATA);++i)data[i]=fgetc(f);
			fclose(f);
			PSAVEDATA save=(PSAVEDATA)data;
			memcpy(bt,&save->bt,sizeof(BESTTIME));
			if(save->exist==0)return FALSE;
			/* LOAD PREVIOUS GAME */
			memcpy(info,&save->mf,sizeof(INFO));
			InitCanvas(canvas,mf,info);
			HMENU hMenu=GetMenu(canvas->hwnd);
			for(i=1;i<=7;++i)CheckMenuItem(hMenu,ID_MULTI+i,i==info->multi?MF_CHECKED:MF_UNCHECKED);
			memcpy(mf,&save->mf,sizeof(MINEFIELD));
			UpdateCounter(canvas,mf->remain);
			canvas->time=save->time-1;
			IncrementTimer(canvas);
			SetTimer(canvas->hwnd,MAIN_TIMER,1000,NULL);
			RepaintField(canvas,mf);
			return info->multi;
		} else fclose(f);
	}
	/* CREATE NEW SAVE FILE UPON FAILURE */
	f=fopen(path,"wb");
	fputc('M',f);
	fputc('M',f);
	fputc('S',f);
	for(i=0;i<1+sizeof(BESTTIME)+sizeof(SHORT)+sizeof(MINEFIELD);++i)fputc(0,f);
	fclose(f);
	/* RETURN DEFAULT BESTTIME */
	memset(bt,0,sizeof(BESTTIME));
	bt->type=TYPE_DEFAULT;
	return FALSE;
}
void SaveFile(PMINEFIELD mf,SHORT time,PBESTTIME bt,char *path) {
	LONG i;
	/* OPEN IN rb+ IN CASE THE FILE HAS BEEN HIDDEN */
	FILE *f=fopen(path,"rb+");
	if(f==NULL)return;
	SAVEDATA save;
	save.h='M';
	save.m='M';
	save.s='S';
	save.exist=mf->progress>0;
	memcpy(&save.bt,bt,sizeof(BESTTIME));
	save.time=time;
	memcpy(&save.mf,mf,sizeof(MINEFIELD));
	char *data=(char*)&save;
	for(i=0;i<sizeof(SAVEDATA);++i)fputc(data[i],f);
	fclose(f);
}
