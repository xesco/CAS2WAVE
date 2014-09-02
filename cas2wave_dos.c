// Author?
// Original Microsoft DOS source

#include <io.h>
#include <fcntl.h>
#include <conio.h>
#include <string.h>


int main(int argc, char *argv[]) {
  int i, n, casfile, wavfile;
  unsigned char m, buffer[128], *b0, *b1;
  long wavlen;

  m = n = wavfile = casfile = 0;
  for (i=1; i<argc; i++) {
    if (!strcmp(argv[i],"-help")) goto showhelp;
    if (!m && !strcmp(argv[i],"-invert")) {
      m = 1;
      continue;
    }
    if (!n && !strcmp(argv[i],"-speed") && i<argc) {
      n = ++i;
      continue;
    }
    if (!wavfile && casfile) wavfile = i;
    if (!casfile) casfile = i;
  }
  if (!casfile) goto showhelp;

  b0 = argv[casfile];
  if (!(b1 = strrchr(b0,'\\'))) {
    if (!(b1 = strrchr(b0,':'))) b1 = b0-1;
  }
  b1++;
  if (strlen(b1) && !strchr(b1,'.') && strlen(b0)<sizeof(buffer)-4) {
    strcpy(buffer,b0);
    strcat(buffer,".cas");
    b0 = buffer;
  }
  casfile = open(b0,O_BINARY | O_RDONLY);
  if (casfile < 0) {
    cputs("Can't open input file\r\n");
    goto exit0;
  }

  if (wavfile) b0 = argv[wavfile];
  if (!(b1 = strrchr(b0,'\\'))) {
    if (!(b1 = strrchr(b0,':'))) b1 = b0-1;
  }
  b1++;
  if (wavfile) {
    if (strlen(b1) && !strchr(b1,'.') && strlen(b0)<sizeof(buffer)-4) {
      strcpy(buffer,b0);
      strcat(buffer,".wav");
      b0 = buffer;
    }
  }
  else {
    b1 = strrchr(b1,'.');
    if (b1 && b1-b0<=strlen(b0)-4) {
      strcpy(b1,".wav");
    }
    else {
      if (b1 && b1-b0<sizeof(buffer)-4) {
	*b1 = '\0';
	strcpy(buffer,b0);
	strcat(buffer,".wav");
	b0 = buffer;
      }
      else b0 = NULL;
    }
  }
  wavfile = open(b0,O_BINARY | O_WRONLY | O_CREAT | O_TRUNC);
  if (wavfile < 0) {
    cputs("Can't open output file\r\n");
    goto exit1;
  }

  b0 = "\226\300\342\370\377\370\342\300\226\152\100\36\10\1\10\36\100\152";
  b1 = "\226\342\377\342\226\100\10\10\100\226\342\377\342\226\100\10\10\100";
  if (n) {
    if (!strcmp(argv[n],"3000")) {
      b0 = "\231\307\352\375\375\352\307\231\147\71\26\3\3\26\71\147";
      b1 = "\261\366\366\261\117\12\12\117\261\366\366\261\117\12\12\117";
    }
    if (!strcmp(argv[n],"3500")) {
      b0 = "\234\317\363\377\363\317\234\144\61\15\1\15\61\144";
      b1 = "\234\363\363\234\61\1\61\234\363\363\234\61\1\61";
    }
    if (!strcmp(argv[n],"4000")) {
      b0 = "\241\332\373\373\332\241\137\46\5\5\46\137";
      b1 = "\300\377\300\100\1\100\300\377\300\100\1\100";
    }
  }

  if (m) {
    for (i=0; i<strlen(b0); i++) b0[i] = -b0[i];
    for (i=0; i<strlen(b1); i++) b1[i] = -b1[i];
  }

  memset(buffer+8,'\200',120);
  if (write(wavfile,"RIFF    WAVEfmt \20\0\0\0\1\0\1\0\104\254\0\0\104\254\0\0\1\0\10\0data    ",0x2c) < 0x2c) goto exit2;
  while (!eof(casfile)) {
    n = read(casfile,buffer,8);

    if (n==8 && !memcmp(buffer,"\37\246\336\272\314\23\175\164",8)) {
      if (tell(casfile) > 8) {
	for (i=200; i; i--) {
	  if (write(wavfile,buffer+8,120) < 120) goto exit2;
	}
      }
      for (i=36000/strlen(b1); i; i--) {
	if (write(wavfile,b1,strlen(b1)) < strlen(b1)) goto exit2;
      }
    }
    else {
      for (i=0; i<n; i++) {
	if (write(wavfile,b0,strlen(b0)) < strlen(b0)) goto exit2;
	for (m=1; m; m<<=1) {
	  if (buffer[i] & m) {
	    if (write(wavfile,b1,strlen(b1)) < strlen(b1)) goto exit2;
	  }
	  else {
	    if (write(wavfile,b0,strlen(b0)) < strlen(b0)) goto exit2;
	  }
	}
	if (write(wavfile,b1,strlen(b1)) < strlen(b1)) goto exit2;
      }
    }
  }
  wavlen = filelength(wavfile);
  wavlen -= 8;
  lseek(wavfile,4,SEEK_SET);
  write(wavfile,&wavlen,4);
  wavlen -= 0x24;
  lseek(wavfile,0x28,SEEK_SET);
  write(wavfile,&wavlen,4);

  close(casfile);
  close(wavfile);
  return 0;

exit2:
  cputs("Can't write output file\r\n");
  close(wavfile);
exit1:
  close(casfile);
exit0:
  return 1;

showhelp:
  cputs("\r\nCAS2WAVE converts MSX tape images into sound data, loadable through the cassete\r\nport of MSX computers. Besides being played directly from PC, sound data in the\r\nWAV files can also be recorded on digital or analog audio media.\r\n\nUsage: cas2wave <input filename> [output filename]\r\nOptions available:\r\n -speed <n>             - Supports 2700 (default), 3000, 3500 and 4000 baud.\r\n -invert                - Inverts the wave.\r\n\nDepending on the player device used, some  speeds may fail to load while higher\r\nones do work fine. Inverting the wave can help too.\r\n");
  return 0;
}
