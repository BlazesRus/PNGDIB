// png2dib.c
// sample using PNGDIB library
// convert a PNG image file to a BMP image file

#include <windows.h>
#include <tchar.h>
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <pngdib.h>

#define P2B_ERROR 1
#define P2B_OK    0

// Read a file into a memory block.
static int read_file_to_mem(const TCHAR *fn,unsigned char **pmem, DWORD *pfsize)
{
	HANDLE hfile=INVALID_HANDLE_VALUE;
	DWORD fsize;
	void *fbuf;
	DWORD bytesread;
	int retval=P2B_ERROR;

	hfile=CreateFile(fn,GENERIC_READ,FILE_SHARE_READ,NULL,
		OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hfile==INVALID_HANDLE_VALUE) goto done;

	fsize=GetFileSize(hfile,NULL);
	if(fsize==INVALID_FILE_SIZE || fsize<=0) goto done;

	fbuf=malloc(fsize);
	if(!fbuf) goto done;

	if(!ReadFile(hfile,fbuf,fsize,&bytesread,NULL)) goto done;
	if(bytesread!=fsize) goto done;

	*pmem  = fbuf;
	fbuf=NULL; // Don't free the memory; it's now owned by the caller.
	*pfsize = fsize;
	retval= P2B_OK;

done:
	if(hfile!=INVALID_HANDLE_VALUE) CloseHandle(hfile);
	if(fbuf) free(fbuf);
	return retval;
}

static int write_dib_to_bmp(const TCHAR *bmpfn, LPBITMAPINFOHEADER pdib, 
					 int dibsize, int bitsoffset)
{
	HANDLE hfile=INVALID_HANDLE_VALUE;
	BITMAPFILEHEADER h;
	DWORD written;
	int retval=P2B_ERROR;

	ZeroMemory((void*)&h,sizeof(h));
	h.bfType= MAKEWORD('B','M');
	h.bfSize=    sizeof(BITMAPFILEHEADER)+dibsize;
	h.bfOffBits= sizeof(BITMAPFILEHEADER)+bitsoffset;

	hfile=CreateFile(bmpfn,GENERIC_WRITE,FILE_SHARE_READ,NULL,
		CREATE_ALWAYS,FILE_ATTRIBUTE_NORMAL,NULL);
	if(hfile==INVALID_HANDLE_VALUE) goto done;

	if(!WriteFile(hfile,(void*)&h,sizeof(BITMAPFILEHEADER),&written,NULL)) {
		goto done;
	}
	if(!WriteFile(hfile,(void*)pdib,dibsize,&written,NULL)) {
		goto done;
	}
	retval=P2B_OK;

done:
	if(hfile!=INVALID_HANDLE_VALUE) CloseHandle(hfile);
	return retval;
}

static int png2bmp_v3(const TCHAR *pngfn, const TCHAR *bmpfn, int frommem)
{
	PNGDIB *pngdib=NULL;
	int ret;
	int retval=P2B_ERROR;
	LPBITMAPINFOHEADER pdib;
	int dib_size;
	void *pdib_bits;
	int bits_offs;
	unsigned char *memblk=NULL;
	int memblksize;
	int need_free_dib=0;

	if(frommem) {
		if(read_file_to_mem(pngfn,&memblk, &memblksize) != P2B_OK) {
			_ftprintf(stderr,_T("Can't read %s\n"),pngfn);
			goto done;
		}
	}

	pngdib = pngdib_p2d_init();
	if(!pngdib) goto done;

	if(frommem) {
		pngdib_p2d_set_png_memblk(pngdib,memblk,memblksize);
	}
	else {
		pngdib_p2d_set_png_filename(pngdib,pngfn);
	}

	//pngdib_p2d_set_gamma_correction(pngdib,PNGDIB_DEFAULT_SCREEN_GAMMA);
	pngdib_p2d_set_use_file_bg(pngdib,1);
	ret=pngdib_p2d_run(pngdib);
	if(ret!=PNGD_E_SUCCESS) {
		_ftprintf(stderr,_T("Failed to load %s: %s (%d)\n"),pngfn,pngdib_get_error_msg(pngdib),ret);
		goto done;
	}
	need_free_dib=1;

	if(frommem) {
		// after pngdib_p2d_run, we can free the memory-mapped PNG
		free(memblk);
		memblk=NULL;
	}

	pngdib_p2d_get_dib(pngdib,&pdib,&dib_size);
	pngdib_p2d_get_dibbits(pngdib,&pdib_bits,&bits_offs,NULL);

	if(write_dib_to_bmp(bmpfn, pdib, dib_size, bits_offs) != P2B_OK) {
		_ftprintf(stderr,_T("Can't write BMP file\n"));
		goto done;
	}

	retval=P2B_OK;

done:
	if(pngdib) {
		// if pngdib_p2d_run() succeeds, then we own the bitmap,
		// and must explicitly free it:
		if(need_free_dib) pngdib_p2d_free_dib(pngdib,NULL);
		pngdib_done(pngdib);
	}
	if(memblk) free(memblk);
	return retval;
}

int _tmain(int argc, TCHAR *argv[])
{
	TCHAR *bmpfn,*pngfn;
	int ret;

	if(argc!=3) {
		_ftprintf(stderr,_T("Usage: %s <file.png> <file.bmp>\n"),argv[0]);
		return 1;
	}
	pngfn=argv[1];
	bmpfn=argv[2];

	ret=png2bmp_v3(pngfn,bmpfn,0);
	//ret=png2bmp_v3(pngfn,bmpfn,1);
	return (ret==P2B_OK)?0:1;
}
