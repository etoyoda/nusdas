/** @file
 * @brief 複合差分圧縮の実装
 */

/*
課題
NuSDaS形式の型に直す
メモリリークのチェック
Endianの挙動のチェック
コメントの追加
暗黙の固定値をヘッダファイルに逃して設定
未初期化変数の初期化
*/
/*
	暗黙の定数
	group_width_nbit=4 (群の幅を格納するビット数)
	data_length=2(byte) (元資料の大きさ)
*/
#include "config.h"
#include "nusdas.h"
#include "internal_types.h"
#include "sys_kwd.h"
# define NEED_PEEK_N_UI4
# define NEED_PEEK_FLOAT
# define NEED_PEEK_DOUBLE
# define NEED_POKE_FLOAT
# define NEED_POKE_DOUBLE
# define NEED_MAKE_UI8
#include "sys_endian.h"
#include "sys_int.h"
#include "glb.h"

#include <string.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include "dset.h"
#include "dfile.h"
#include "sys_file.h"
#include "ndf_codec.h"
#include "sys_err.h"
# define NEED_MEMCPY4
# define NEED_MEMCPY8
#include "sys_string.h"
#include "sys_mem.h"

#include <stdio.h>
#ifdef HAVE_LRINT
# define ROUND(x)	lrint(x)
#else
# define ROUND(x)	floor((x) + 0.5)
#endif

/* nus_decode_cpsd の第3引数lenは実は全く利用されない */
long
nus_decode_cpsd(const unsigned char *src, unsigned char *dst, N_UI4 len) {
	N_SI4 group_num,org_size;
	N_SI4 i,j,k;
	N_UI2 *group_ref;
	N_UI2 *group_width;
	N_SI4 *group_adr;
	unsigned char d;
	N_UI2 *unpack;
	N_SI4 *pack;
/* 以下はヘッダファイルで纏めたほうが良いかもしれない*/
	const N_SI4 group_size=32;
	const N_SI4 pack_offset = 32768;/*通常で行うと差分の値が0近傍に分布するためoffsetを加えておく*/
	N_SI4 off=0;/* 元データの読み込み開始位置*/

	org_size=PEEK_N_UI4(src+off);
	off +=4;
	group_num=(org_size-1)/group_size +1;
	group_ref = malloc(sizeof(N_UI2) * group_num);
	group_width = malloc(sizeof(N_UI2) * (group_num+1));
	group_adr = malloc(sizeof(N_SI4) * group_num);
	pack = malloc(sizeof(N_SI4) * (org_size+3));
	unpack = malloc(sizeof(N_UI2) * org_size);
/*要endiean対応*/
#ifdef WORDS_BIGENDIAN
	memcpy(group_ref,src+off,group_num*2);
#else
	for(i=0;i<group_num;i++){
		memcpy((unsigned char*)group_ref+i*2+1,src+i*2+off,1);
		memcpy((unsigned char*)group_ref+i*2,src+i*2+1+off,1);
	}
#endif
	off += (group_num*2);
	/*poption parallel tlocal(i,d) */
	for(i=0;i<((group_num-1)/2+1);i++){
		memcpy(&d,src+i+off,1);
		group_width[i*2]=d >>4;
		group_width[i*2+1]=d & 0x0F;
/*群の幅は4ビットに抑えるため1引いた値で格納されているので、1加算する*/
		group_width[i*2] +=1;
		group_width[i*2+1] +=1;
	}
	off += ((group_num-1)/2 + 1);
/*
	並列化のために各グループの先頭アドレスを求める
*/
	for(i=0;i<group_num;i++){
		group_adr[i]=off;
		off += (group_width[i]*4);
	}
#ifdef USE_OMP
#pragma omp parallel for private(i,j,k)
#else
	/*poption parallel */
#endif
	for(i=0;i<group_num;i++){
		N_SI4 pos=0;
		N_SI4 pos_bit=0;
		N_UI4 diff_v,diff_v1;
		N_SI4 start_p,end_p;

		start_p=i*group_size;
		end_p=org_size;
		if (i < (group_num -1 )){
			end_p=start_p + group_size;
		}
		diff_v = PEEK_N_UI4(src+group_adr[i]);
		for(j=start_p;j<end_p;j++){
			if ((pos_bit+group_width[i]) > 32){
				diff_v1=(( diff_v <<  pos_bit) >> (32 - group_width[i])) ;
				pos += 4;
				diff_v = PEEK_N_UI4(src+group_adr[i]+pos);
				pack[j]=(diff_v1 | ( diff_v >> (64 - (pos_bit + group_width[i]))) ) +group_ref[i];
				pos_bit = pos_bit+group_width[i] - 32;
			}else if ((pos_bit+group_width[i]) == 32) {
				pack[j]=(( diff_v <<  pos_bit) >> (32 - group_width[i]))  +group_ref[i];
				pos += 4;
				diff_v = PEEK_N_UI4(src+group_adr[i]+pos);
				pos_bit = 0;
			} else{
				pack[j]=(( diff_v <<  pos_bit) >> (32 - group_width[i])) +group_ref[i] ;
				pos_bit = pos_bit+group_width[i];
			}
		}
	}
	for(i=0;i<org_size;i++){
		pack[i] -= pack_offset;
		if ( pack[i] < 0 ){
			pack[i] += 65536;
		}
	}
	unpack[0]=pack[0];
	unpack[1]=pack[1];
/* 16bitの範囲に収まるようにpackを循環させる*/
	for(i=2;i<org_size;i++){
		unpack[i]=( 2*unpack[i-1] ) - unpack[i-2] + pack[i];
	}
	for(i=0;i<org_size;i++){
		POKE_N_UI2(dst+i*2,unpack[i]);
	}
	free(group_ref);
	free(group_width);
	free(group_adr);
	free(pack);
	free(unpack);
	return (sizeof(N_UI2) * org_size);
}

/* nus_encode_cpsd の第5引数dlenは実は全く利用されない */
long
nus_encode_cpsd(const unsigned char *src, N_UI4 x, N_UI4 y, unsigned char *dst, N_UI4 dlen) {

	N_SI4 i,j;
	N_SI4 group_num;
	N_UI2 *unpack;
	N_UI2 *pack;
	N_SI4 org_size;
	N_UI2 *group_ref;
	N_UI2 *group_width;
	N_UI4 *raw;
	N_SI4 *raw_pos;

/* 以下はヘッダファイルで纏めたほうが良いかもしれない*/
	const N_SI4 group_size=32;
	const N_SI4 pack_offset = 32768;/*通常で行うと差分の値が0近傍に分布するためoffsetを加えておく*/
	N_SI4 off=0;/* 元データの読み込み開始位置*/

	org_size= x * y;
	group_num=((org_size-1)/group_size+1);
	unpack = malloc(sizeof(N_UI2) * org_size);
	pack = malloc(sizeof(N_UI4) * org_size);
	group_ref = malloc(sizeof(N_UI2) * group_num);
	group_width = malloc(sizeof(N_UI2) * group_num);
	raw = malloc(sizeof(N_UI4) * org_size);
	raw_pos = malloc(sizeof(N_SI4) * org_size);
#ifdef WORDS_BIGENDIAN
	memcpy(unpack,src,org_size*2);
#endif
	for (i=0;i<(org_size);i++){
//		memcpy(unpack+i,src+2*i+off,2);
#ifndef WORDS_BIGENDIAN
		memcpy((unsigned char*)unpack+i*2+1,src+i*2,1);
		memcpy((unsigned char*)unpack+i*2,src+i*2+1,1);
#endif
		raw[i]=0;
	}
/* 2階差分を行う */
	pack[0] = unpack[0] + pack_offset;
	pack[1] = unpack[1] + pack_offset;
	/*poption parallel */
	for (i=2;i<(org_size);i++){
/* 16bitの範囲に収まるようにpackを循環させる*/
		pack[i] = unpack[i] - ( 2 * unpack[i-1]) + unpack[i-2];
		pack[i] += pack_offset;
	}
#ifdef USE_OMP
#pragma omp parallel for private(i,j)
#else
	/*poption parallel */
#endif
	for (i=0;i<group_num;i++){
		N_SI4 k,max_v,min_v;
		N_SI4 diff_b=0;
		N_UI4 diff_v;
		N_SI4 pos=0;
		N_SI4 pos_bit=0;
		N_SI4 start_p,end_p;
		N_SI4 shift;
		min_v=65535;
		max_v=0;
		start_p=i*group_size;
		end_p=org_size;
		if (i < (group_num -1 )){
			end_p=start_p + group_size;
		}
		for (j=start_p;j<end_p;j++){
			if (max_v < pack[j]) {max_v = pack[j];}
			if (min_v > pack[j]) {min_v = pack[j];}
		}
		diff_v= (max_v - min_v);
		while (diff_v != 0){
			diff_b ++;
			diff_v = diff_v >> 1;
		}
/*群の幅は4ビットに抑えるため1引いた値で格納*/
		if (diff_b != 0){
			diff_b -=1;
		}
		group_width[i]=diff_b;
/*この後の計算は元のビット幅で行う*/
		diff_b += 1;
		group_ref[i]=min_v;
//		if (diff_b > 0) {
			start_p=i*group_size;
			pos=start_p;
			end_p=org_size;
			if (i < (group_num -1 )){
				end_p=start_p + group_size;
			}
			for (j=start_p;j<end_p;j++){
				diff_v=pack[j] - min_v;
				if ((shift=(pos_bit+diff_b)) > 32){
					raw[pos]=raw[pos] | (diff_v >> (diff_b - (32 - pos_bit)));
					pos ++;
					raw[pos]= (diff_v << (64 - shift));
					pos_bit = shift - 32;
				} else{
					raw[pos]=raw[pos] | (diff_v << (32 - shift));
					pos_bit += diff_b;
					if (pos_bit == 32 ){
						pos += 1;
						pos_bit =0;
					}
				}
			}
//		}
		raw_pos[i]=pos;
		if (pos_bit !=0){
			raw_pos[i] ++;
		}
	}
/* Output*/
	POKE_N_SI4(dst+off,org_size);
	off +=4;
	/*poption parallel */
	for (i=0;i<group_num;i++){
		POKE_N_UI2(dst+off+2*i,group_ref[i]);
	}
	off +=(2*group_num);
	for (i=0;i<(group_num-1);i=i+2){
		dst[off+i/2]=group_width[i]<< 4 | group_width[i+1];
	}
/*余りビット処理*/
	if ((group_num % 2 ) == 1){
		dst[off+(group_num/2)] = group_width[group_num-1] <<4;
	}
	off += ((group_num-1) / 2 + 1);
	for (i=0;i<group_num;i++){
/*圧縮されたデータは32bit単位になっているはず*/
		for (j=i*group_size;j<raw_pos[i];j++){
			POKE_N_UI4(dst+off,raw[j]);
			off += 4;
		}
	}
	free(unpack);
	free(pack);
	free(group_ref);
	free(group_width);
	free(raw);
	free(raw_pos);
	return (off);
}

#if 0
int main(int argc, char *argv[]){
	N_UI2 *src,*t;
	unsigned char *d;
	N_SI4 i,dlen,rc,j;
	N_UI4 x,y;
	char str[100];
	char c[10];
	
	x=65;
	y=1;
//	src = malloc(sizeof(short) * x*y);
//	t = malloc(sizeof(short) * x*y);
	for (i=0;i<x*y;i++){
//単調増加
//		src[i]=i;
//一定値
//		src[i]=257;
//がっこんがっこん
/*		if ((i%2)==0){
			src[i]=0;
		}else{
			src[i]=65534;
			src[i]=17;
		}*/
//適当
/*		if (i==0){
			src[i]=10;
		}else{
			src[i]=src[i-1]*i+i;
		}
		if (src[i] >32767){
			src[i] -= 32768;
		}*/
	}
//実データ
	FILE *fp;
	if (argc<2){
		printf("a.out NuSDaS_File\n");
		exit(0);
	}
    if ((fp=fopen(argv[1],"r"))==NULL){
		 printf("Nusdas File can not open. !!!\n");
	     exit (1);
    }
	while ((j=fread(str,1,1,fp)) == 1){
		c[0]='D';
		if (strncmp(str,c,1) == 0){
			fread(str,29,1,fp);
			c[0]='A';
			c[1]='T';
			c[2]='A';
			if (strncmp(str,c,3)!= 0){ continue;}
			c[0]='S';
			c[1]='U';
			c[2]='R';
			c[3]='F';
			fread(str,6,1,fp);
			if (strncmp(str,c,4)!= 0){ continue;}
			fread(str,8,1,fp);
			c[0]='P';
			c[1]='S';
			c[2]='E';
			c[3]='A';
			if (strncmp(str,c,4)!= 0){ continue;}
			fread(str,4,1,fp);
			x=PEEK_N_UI4(str);
			fread(str,4,1,fp);
			y=PEEK_N_UI4(str);
			printf("OK %d %d\n",x,y);
			fread(str,16,1,fp);
			src = malloc(sizeof(short) * x*y);
			t = malloc(sizeof(short) * x*y);
			d = malloc(sizeof(short) * x*y);
			fread((unsigned char*)src,x,y,fp);
			break;
		}
	}
	fclose(fp);
//	d = malloc(10000);
	rc=encode_jpeg2000((const unsigned char *)src, x, y, d, dlen);
	printf("rc=%d\n",rc);
	printf("*** Pack ***\n",rc);
	for (i=0;i<(rc-1)/16+1;i++){
		for (j=0;j<16;j++){
			if ((i*16+j) >=rc){break;}
			printf("%d ",d[i*16+j]);
		}
		printf("\n",rc);
	}
	decode_jpeg2000((const unsigned char*) d, (unsigned char *)t, rc);
	printf("*** Result ***\n",rc);
	for (i=0;i<x*y;i++){
		if ((src[i]-t[i])==0){
			printf("OK %d %d %d %d\n",i,src[i],t[i],src[i]-t[i]);
		}else{
			printf("NG %d %d %d %d\n",i,src[i],t[i],src[i]-t[i]);
		}
	}
	printf("Unpack size=%d\n",x*y*2);
	printf("Pack   size=%d\n",rc);
	return (0);
}
#endif
