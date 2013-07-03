#include "./../include/stdlib.h"
#include "./../include/sheet.h"
#define SHEET_USED 1
struct SHTCTL*shtctl_init(unsigned char * vram, int xsize, int ysize)	
{
	struct SHTCTL* ctl;
	int i;
	ctl = (struct SHTCTL*)malloc(sizeof(struct SHTCTL));//因为这个结构很大所以我们最好通过动态分配内存，以防止栈溢出
	if(ctl == 0)
	{
		return ctl;
	}
	
	ctl->map = (unsigned char*)malloc(MAP_PAGE_SIZE);//我们申请大的内存,这个内存描述界面图层的格局
	if(0 == ctl->map)
	{
		return (struct SHTCTL*)0;
	}
		
	
	ctl->vram = vram;
	ctl->xsize = xsize;
	ctl->ysize = ysize;
	ctl->top = -1;
	for(i = 0; i < MAX_SHEETS; i++)
	{
		ctl->sheet0[i].flags = 0;//标记未使用
	}
	return ctl;
}
//查找空闲的图层
struct SHEET* sheet_alloc(struct SHTCTL* ctl)
{	
	int i;
	for(i = 0; i < MAX_SHEETS; i++)
	{
		if(ctl->sheet0[i].flags == 0)
		{
			ctl->sheet0[i].flags = SHEET_USED;
			ctl->sheet0[i].height = -1;
			return &(ctl->sheet0[i]);
		}
	}
	return (struct SHEET*)0;
}
void sheet_setbuf(struct SHEET*sht, unsigned char * buf, int xsize, int ysize, int col_inv)
{
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize; 
	sht->col_inv = col_inv;
}

void sheet_updown(struct SHTCTL *ctl, struct SHEET* sht, int height)
{
	int h,  old = sht->height;//保存高度
	if(height > ctl->top + 1)//修正设置的高度
		height = ctl->top + 1;
	if(height < -1)	
		height = - 1;
		
	sht->height = height;
	if(old > height)//如果往下移动了，我们把该图层之下的往上提
	{
		if(height >= 0)
		{
			for(h = old; h > height; h--)
			{
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
				}
			ctl->sheets[height] = sht;
		}else
		{
			if(ctl->top > old)
			{
				for(h = old; h < ctl->top; h++)
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
			}
			ctl->top--;
		}
		sheet_refreshsub(ctl,sht->vx0,sht->vy0,sht->bxsize,sht->bysize,0);
		
	}else//如果往上移动，我们把之上的往下移动
	{
		if(old > 0)
		{
			for(h = old; h < height; h++)
			{
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
		}else
		{
			for(h = ctl->top; h >= height; h--)
			{
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			
			ctl->sheets[height] = sht;
			ctl->top++;//由于我们新插入了一个图层我们需要增加top
		}
	sheet_refreshsub(ctl,sht->vx0,sht->vy0,sht->bxsize,sht->bysize,sht->height);
	}
	
}
//刷新图层
void sheet_refresh(struct SHTCTL *ctl, struct SHEET*sht, int bx0, int by0, int bx1, int by1)
{
	 if(sht->height >= 0)
	{
		sheet_refreshsub(ctl, sht->vx0+bx0,sht->vy0+by0, sht->vx0+bx1, sht->vy0+by1,sht->height);
	}
}

//图层移到的位置
void sheet_slide(struct SHTCTL *ctl,struct SHEET* sht, int vx, int vy)
{
	int oldx = sht->vx0, oldy = sht->vy0;
	sht->vx0 = vx;
	sht->vy0 = vy;
	if(sht->height >= 0)
	{
		sheet_refreshsub(ctl, oldx, oldy, oldx + sht->bxsize, oldy + sht->bysize,0);
		sheet_refreshsub(ctl, vx,    vy,  vx  +  sht->bxsize, vy   + sht->bysize,sht->height);
	}
}

void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1,int h0)
{
	int h, x0, y0,x1,y1,x,y;
	struct SHEET *sht;
	unsigned char *vrambuf,*buf,c, *vram = ctl->vram;
	 for(h = h0; h <= ctl->top; h++)
	 {
		sht = ctl->sheets[h];
		x0 = vx0 - sht->vx0;
		y0 = vy0 - sht->vy0;
		if(x0 < 0)
			x0 = 0;
		if(y0 < 0)
			y0 = 0;
		x1 = vx1 - sht->vx0;
		y1 = vy1 - sht->vy0;
		if(x1 > sht->bxsize) x1 = sht->bxsize;
		if(y1 > sht->bysize) y1 = sht->bysize;
		
		for(y = y0; y < y1; y++)
		{
			vrambuf = &vram[(y + sht->vy0)*ctl->xsize + sht->vx0];
			buf = &sht->buf[y * sht->bxsize];
			for(x = x0; x < x1; x++)
			{
				c = buf[x];
				if(c != sht->col_inv)
					vrambuf[x] = c;
			}
		}
	}	
		
				
	
	
}
void sheet_refreshmap(struct SHTCTL*ctl, int vx0, int vy0, int vx1, int vy1, int h0)
{

}
//释放该页面
void sheet_free(struct SHEET *sht)
{
	sht->flags = 0;
}
