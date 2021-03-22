/*
浮動小数点データ圧縮プログラム

*** サブルーチンリスト
int decode_r8 : r8デコード(C)
int decode_r4 : r4デコード(C)
int encode_r8 : r8エンコード(C)
int encode_r4 : r4エンコード(C)
void DECODE_R8 : r8デコード(Fortran)
void DECODE_R4 : r4デコード(Fortran)
void ENCODE_R8 : r8エンコード(Fortran)
void ENCODE_R4 : r4エンコード(Fortran)

*** (以下開発者向け)
*** 圧縮内部フォーマット
1-4byte　資料の数
5-8　圧縮後の大きさ
9　flag1
10　flag2(R4は未使用)
11-14 指数部の先頭アドレス
15-18 仮数部1の先頭アドレス
19-22 仮数部2の先頭アドレス
23-26 仮数部3の先頭アドレス(R4の場合無し)
27-30 仮数部4の先頭アドレス(R4の場合無し)
31(23)-　符号部のデータ
x0-　指数部のデータ
x1-　仮数部のデータ1
x2-　仮数部のデータ2
x3-  仮数部のデータ3 (R4の場合無し)
x4-  仮数部のデータ4 (R4の場合無し)

***　各部がランレングス圧縮されていた場合のフォーマット
0-3 1bit目のランレングス圧縮サイズ(byte)
4-  2bit目のランレングス圧縮サイズ(byte)
(以降、nbit目まで繰り返し)
4n-   1bit目のランレングスデータ
4n+x1 2bit目のランレングスデータ
***　ランレングス圧縮のフォーマット
0-1bit　最初のデータの値(0or1)
2-　　　(2bit単位)データ

データは同じデータが続く長さ-2,反転符号(11(2進数))の順番で並べる
長さは3進法で下位から記載する。
単に反転する場合は長さは省略する

例:18個同じデータが続く場合
18-2=16=1*1+2*3+1*9=010201(02) となる

例:データが0101と続く場合
11 11 11 となる

***　複合差分圧縮圧縮されていた場合のフォーマット
0-group_num/8*(7 or 8 or 11 or 16) 群の参照値
x-group_num/2 群の幅
y-参照値からの差分(群の幅により大きさは異なる)
  (群の幅が0の場合差分は省略される)

*** flag1の詳細
0bit-　符号部を圧縮するか
2-3　指数部を圧縮するか　10:ランレングス圧縮　01:複合差分圧縮　00:非圧縮
4-5　仮数部1を圧縮するか　10:ランレングス圧縮　01:複合差分圧縮　00:非圧縮
6-7　仮数部2を圧縮するか　10:ランレングス圧縮　01:複合差分圧縮　00:非圧縮

*** flag2の詳細
0-1　仮数部3を圧縮するか　10:ランレングス圧縮　01:複合差分圧縮　00:非圧縮
2-3　仮数部4を圧縮するか　10:ランレングス圧縮　01:複合差分圧縮　00:非圧縮

*** R8のデータ分割
0bit　符号部
1-11  指数部
12-27 仮数部1
28-43 仮数部2
44-59 仮数部3
60-63 仮数部4

*** R4のデータ分割
0bit　符号部
1-8  指数部
9-15 仮数部1
16-31 仮数部2

***    動作用　プリプロセッサ一覧
    OMP openMPの指示符を付与する
    SD  2階差分を使う(デフォルトは1階)
***    デバッグ用　プリプロセッサ一覧
    Test : mainプログラムとして動作させる
    R4   : R4のテストを行う(デフォルトはR8)
    S_RUN : 符号部を強制的に圧縮する
    S_NO : 符号部を圧縮しない
    E_RUN :指数部を強制的にランレングス圧縮する
    E_COM :指数部を強制的に複合差分圧縮する
    E_NO  :指数部を圧縮しない
    F_RUN :仮数部を強制的にランレングス圧縮する
    F_COM :仮数部を強制的に複合差分圧縮する
    F_NO  :仮数部を圧縮しない
    READ_RAW 生データを読み込む
    READ_DECODE　生データをpack後、アンパックし検証する(デフォルトは生データをPackのみ)
    READ_PACK Packされたデータを読みデコードする
    MAKE_DUMMY ダミーの生データを作る
    DEBUG : 大量に出るデバッグコードを吐き出す(実際にはこれに加えてdebug=2の指定が必要)

*/

#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <memory.h>
#include <string.h>
#include <time.h>
#include <sys/time.h>
#ifdef OMP
#include <omp.h>
#endif
/* #ifdef Little_Endian -- nusdasのconfigureで元々存在したBIGENDIAN判定configを使うように変更*/
#ifndef WORDS_BIGENDIAN
# define HTON2(i16) \
    ( (((i16) & 0xFF00) >> 8) \
    | (((i16) &   0xFF) << 8))
# define HTON4(i32) \
    ( (((i32) & 0xFF000000uL) >> 24) \
    | (((i32) &   0xFF0000uL) >> 8) \
    | (((i32) &     0xFF00uL) << 8) \
    | (((i32) &       0xFFuL) << 24))
# define NTOH4(i32) \
    ( (((i32) & 0xFF000000uL) >> 24) \
    | (((i32) &   0xFF0000uL) >> 8) \
    | (((i32) &     0xFF00uL) << 8) \
    | (((i32) &       0xFFuL) << 24))
#else
# define HTON4(i32) (i32)
# define NTOH4(i32) (i32)
# define HTON2(i16) (i16)
#endif

#if NEED_ALIGN & 4
void
poke4(unsigned char *ptr, unsigned int val)
{
    ptr[0] = ((val >> 24) & 0xFF);
    ptr[1] = ((val >> 16) & 0xFF);
    ptr[2] = ((val >> 8) & 0xFF);
    ptr[3] = (val & 0xFF);
}
unsigned int
peek4(const unsigned char *ptr)
{
    return (unsigned int)ptr[0] << 24
        | (unsigned int)ptr[1] << 16
        | (unsigned int)ptr[2] << 8
        | (unsigned int)ptr[3];
}
# define POKE_N_SI4(ptr, val) poke4((unsigned char *)(ptr), val)
# define POKE_N_UI4(ptr, val) poke4((unsigned char *)(ptr), val)
# define PEEK_N_UI4(ptr) peek4((const unsigned char *)(ptr))
#else
# define POKE_N_SI4(ptr, val) (*(unsigned int *)(ptr) = HTON4((unsigned int)(val)))
# define POKE_N_UI4(ptr, val) (*(unsigned int *)(ptr) = HTON4((unsigned int)(val)))
# define PEEK_N_UI4(ptr) NTOH4(*(unsigned int *)(ptr))
#endif
#if NEED_ALIGN & 2
void
poke2(unsigned char *ptr, unsigned short val)
{
    ptr[0] = ((val >> 8) & 0xFF);
    ptr[1] = (val & 0xFF);
}
unsigned short 
peek2(const unsigned char *ptr)
{
    return (unsigned short)ptr[0] << 8 | (unsigned short)ptr[1];
}
# define POKE_N_UI2(ptr, val) poke2((unsigned char *)(ptr), val)
# define PEEK_N_UI2(ptr) peek2((const unsigned char *)(ptr))
#else
# define POKE_N_UI2(ptr, val) (*(unsigned short *)(ptr) = HTON2((unsigned short)(val)))
# define PEEK_N_UI2(ptr) HTON2(*(unsigned short *)(ptr))
#endif
#define RUNLENGTH_LIMIT (5.0) /*ランレングス圧縮を採用するリミット(%)*/

int debug = 1;
/* 以下はヘッダファイルで纏めたほうが良いかもしれない*/
const int group_size=32; /*32の倍数であること*/
const int pack_offset16 = 32768;/*通常で行うと差分の値が0近傍に分布するためoffsetを加えておく(参照値16bit用)*/
const int pack_offset11 = 1024;/*通常で行うと差分の値が0近傍に分布するためoffsetを加えておく(参照値11bit用)*/
const int pack_offset8 = 128;/*通常で行うと差分の値が0近傍に分布するためoffsetを加えておく(参照値8bit用)*/
const int pack_offset7 = 64;/*通常で行うと差分の値が0近傍に分布するためoffsetを加えておく(参照値7bit用)*/

int decode_runlength(unsigned char * src,unsigned char * dst,int org_size,int org_byte_size,int org_byte_off,int org_bit_off){
    int bit_pos=0;
    int total_size=0;
    int off=0;
    int i;
    unsigned char old_sign;
/*    int v;*/
    int sign_count =2;
    int place=0;
    int old_v=3;
    int vv[4];
    int j;
    const unsigned char mask2[8]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
    const int pow_table[20]={    1,    3,    9,    27,    81,
                               243,  729, 2187,  6561, 19683,
                             59049,177147,531441,1594323,4782969,
                          14348907,43046721,129140163,387420489,1162261467};
    old_sign = (src[off] >>6) << (7-org_bit_off);
    vv[1] = (src[off] >>4) & 0x03;
    vv[2] = (src[off] >>2) & 0x03;
    vv[3] = src[off] & 0x03;
    off++;
/*org_sizeが10以下だとうまくいかないので注意*/
    for(j=1;j<4;j++){
        if (vv[j] != 3){
            sign_count += vv[j]*pow_table[place];
            place ++;
        }else{
            if (old_v != 3){
                for(i=total_size*org_byte_size+org_byte_off;i<(total_size+sign_count)*org_byte_size+org_byte_off;i+=org_byte_size){
                    dst[i] = old_sign ;
                }
                total_size += sign_count;
            }else{
                dst[total_size*org_byte_size+org_byte_off] = old_sign ;
                total_size ++;
            }
            sign_count=2;
            old_sign ^= mask2[org_bit_off];
            place = 0;
        }
        old_v =vv[j];
    }
    while (1){
        vv[0] = src[off] >>6;
        vv[1] = (src[off] >>4) & 0x03;
        vv[2] = (src[off] >>2) & 0x03;
        vv[3] = src[off] & 0x03;
        off++;
/* やっているのは以下のコードだが、ループの手動アンローディングをすると爆速になるのでしょうがなく汚く書く
(iccではどうしてもやってくれない)
        for(j=1;j<4;j++){
            if (vv[0] == 3){
                if (old_v == 3){
                    dst[total_size*org_byte_size+org_byte_off] = old_sign ;
                    total_size ++;
                }else{
                    for(i=total_size*org_byte_size+org_byte_off;i<(total_size+sign_count)*org_byte_size+org_byte_off;i+=org_byte_size){
                        dst[i] = old_sign ;
                    }
                    total_size += sign_count;
                }
                sign_count=2;
                old_sign ^= mask2[org_bit_off];
                place = 0;
            }else{
                sign_count += vv[0]*pow_table[place];
                place ++;
            }
            old_v =vv[0];
            if (total_size >= org_size){ goto OUT1;}
        }
OUT1:
*/
        if (vv[0] != 3){
            sign_count += vv[0]*pow_table[place];
            place ++;
        }else{
            if (old_v == 3){
                dst[total_size*org_byte_size+org_byte_off] = old_sign ;
                total_size ++;
            }else{
                for(i=total_size*org_byte_size+org_byte_off;i<(total_size+sign_count)*org_byte_size+org_byte_off;i+=org_byte_size){
                    dst[i] = old_sign ;
                }
                total_size += sign_count;
            }
            sign_count=2;
            old_sign ^= mask2[org_bit_off];
            place = 0;
            if (total_size >= org_size){ break;}
        }
        if (vv[1] != 3){
            sign_count += vv[1]*pow_table[place];
            place ++;
        }else{
            if (vv[0] == 3){
                dst[total_size*org_byte_size+org_byte_off] = old_sign ;
                total_size ++;
            }else{
                for(i=total_size*org_byte_size+org_byte_off;i<(total_size+sign_count)*org_byte_size+org_byte_off;i+=org_byte_size){
                    dst[i] = old_sign ;
                }
                total_size += sign_count;
            }
            if (total_size >= org_size){ break;}
            sign_count=2;
            old_sign ^= mask2[org_bit_off];
            place = 0;
        }
        if (vv[2] != 3){
            sign_count += vv[2]*pow_table[place];
            place ++;
        }else{
            if (vv[1] == 3){
                dst[total_size*org_byte_size+org_byte_off] = old_sign ;
                total_size ++;
            }else{
                for(i=total_size*org_byte_size+org_byte_off;i<(total_size+sign_count)*org_byte_size+org_byte_off;i+=org_byte_size){
                    dst[i] = old_sign ;
                }
                total_size += sign_count;
            }
            if (total_size >= org_size){ break;}
            sign_count=2;
            old_sign ^= mask2[org_bit_off];
            place = 0;
        }
        if (vv[3] != 3){
            sign_count += vv[3]*pow_table[place];
            place ++;
        }else{
            if (vv[2] == 3){
                dst[total_size*org_byte_size+org_byte_off] = old_sign ;
                total_size ++;
            }else{
                for(i=total_size*org_byte_size+org_byte_off;i<(total_size+sign_count)*org_byte_size+org_byte_off;i+=org_byte_size){
                    dst[i] = old_sign ;
                }
                total_size += sign_count;
            }
            if (total_size >= org_size){ break;}
            sign_count=2;
            old_sign ^= mask2[org_bit_off];
            place = 0;
        }
        old_v =vv[3];
    }
    return(off);
}
/*非圧縮を戻す*/
int nopack_restore(const unsigned char* src, /*入力配列*/
                   unsigned char* dst, /*出力配列*/
                   int size,/*入力配列の数*/
                   int byte_off,/*入力配列のオフセット*/
                   int bit_size,/*入力配列のビット数*/
                   int byte_size/*出力配列の大きさ*/
                   ){
    int add=0;
    int off=0;
    int i,j;
    int end_p;

    if (bit_size==11){
/*R8 仮数部1の処理*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<size/8;i++){
            dst[i*8*8]       =dst[i*8*8] | ((src[i*11] >> 1) & 0x7f );
            dst[i*8*8+1]     =(src[i*11] << 7 ) | ((src[i*11 +1 ] >> 1) & 0x70);
            dst[(i*8+1)*8]   =dst[(i*8+1)*8] | ((src[i*11+1] << 2 ) & 0x7f) | (src[i*11+2] >> 6 );
            dst[(i*8+1)*8+1] =((src[i*11 +2 ] << 2) & 0xf0);
            dst[(i*8+2)*8]   =dst[(i*8+2)*8] | ((src[i*11+2] << 5 ) & 0x7f) | (src[i*11+3] >> 3 );
            dst[(i*8+2)*8+1] =((src[i*11 +3 ] << 5) & 0xf0) | ((src[i*11 +4 ] >> 3) & 0x10);
            dst[(i*8+3)*8]   =dst[(i*8+3)*8] | (src[i*11+4]  & 0x7f) ;
            dst[(i*8+3)*8+1] =(src[i*11 +5 ] & 0xf0);
            dst[(i*8+4)*8]   =dst[(i*8+4)*8] | ((src[i*11+5] << 3 ) & 0x7f) | (src[i*11+6] >> 5 );
            dst[(i*8+4)*8+1] =((src[i*11 + 6 ] << 3) & 0xf0);
            dst[(i*8+5)*8]   =dst[(i*8+5)*8] | ((src[i*11+6] << 6 ) & 0x7f) | (src[i*11+7] >> 2 );
            dst[(i*8+5)*8+1] =(src[i*11 +7 ] << 6) | ((src[i*11 +8 ] >> 2) & 0xf0);
            dst[(i*8+6)*8]   =dst[(i*8+6)*8] | ((src[i*11+8] << 1 ) & 0x7f) | (src[i*11+9] >> 7 );
            dst[(i*8+6)*8+1] =((src[i*11 +9 ] << 1) & 0xf0);
            dst[(i*8+7)*8]   =dst[(i*8+7)*8] | ((src[i*11+9] << 4 ) & 0x7f) | (src[i*11+10] >> 4 );
            dst[(i*8+7)*8+1] =((src[i*11 +10 ] << 4) & 0xf0);
        }
        off += (((int) size/8) *11 );
/*余りの処理*/
        i = (((int) size/8) *8);
        if ( i < size){
            dst[i*8]       =dst[i*8] | ((src[off] >> 1) & 0x7f );
            dst[i*8+1]     =(src[off] << 7 ) | ((src[off +1 ] >> 1) & 0x70);
            add =2;
        }
        if ( i+1 < size){
            dst[(i+1)*8]   =dst[(i+1)*8] | ((src[off+1] << 2 ) & 0x7f) | (src[off+2] >> 6 );
            dst[(i+1)*8+1] =((src[off+2 ] << 2) & 0xf0);
            add =3;
        }
        if ( i+2 < size){
            dst[(i+2)*8]   =dst[(i+2)*8] | ((src[off+2] << 5 ) & 0x7f) | (src[off+3] >> 3 );
            dst[(i+2)*8+1] =((src[off+3 ] << 5) & 0xf0) | ((src[off+4 ] >> 3) & 0x10);
            add =5;
        }
        if ( i+3 < size){
            dst[(i+3)*8]   =dst[(i+3)*8] | (src[off+4]  & 0x7f) ;
            dst[(i+3)*8+1] =(src[off +5 ] & 0xf0);
            add =6;
        }
        if ( i+4 < size){
            dst[(i+4)*8]   =dst[(i+4)*8] | ((src[off+5] << 3 ) & 0x7f) | (src[off+6] >> 5 );
            dst[(i+4)*8+1] =((src[off + 6 ] << 3) & 0xf0);
            add =7;
        }
        if ( i+5 < size){
            dst[(i+5)*8]   =dst[(i+5)*8] | ((src[off+6] << 6 ) & 0x7f) | (src[off+7] >> 2 );
            dst[(i+5)*8+1] =(src[off +7 ] << 6) | ((src[off +8 ] >> 2) & 0xf0);
            add =9;
        }
        if ( i+6 < size){
            dst[(i+6)*8]   =dst[(i+6)*8] | ((src[off+8] << 1 ) & 0x7f) | (src[off+9] >> 7 );
            dst[(i+6)*8+1] =((src[off +9 ] << 1) & 0xf0);
            add =10;
        }
        off += add;
    }else if (bit_size==7){
/*R4 仮数部1の処理*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<size/8;i++){
            dst[i*8*4+1]     =  dst[i*8*4+1] | (src[i*7] >> 1 );
            dst[(i*8+1)*4+1]   =dst[(i*8+1)*4+1] | (((src[i*7] << 6 ) | (src[i*7+1] >> 2 )) & 0x7f);
            dst[(i*8+2)*4+1]   =dst[(i*8+2)*4+1] | (((src[i*7+1] << 5 ) | (src[i*7+2] >> 3 )) & 0x7f);
            dst[(i*8+3)*4+1]   =dst[(i*8+3)*4+1] | (((src[i*7+2] << 4 ) | (src[i*7+3] >> 4 )) & 0x7f);
            dst[(i*8+4)*4+1]   =dst[(i*8+4)*4+1] | (((src[i*7+3] << 3 ) | (src[i*7+4] >> 5 )) & 0x7f);
            dst[(i*8+5)*4+1]   =dst[(i*8+5)*4+1] | (((src[i*7+4] << 2 ) | (src[i*7+5] >> 6 )) & 0x7f);
            dst[(i*8+6)*4+1]   =dst[(i*8+6)*4+1] | (((src[i*7+5] << 1 ) | (src[i*7+6] >> 7 )) & 0x7f);
            dst[(i*8+7)*4+1]   =dst[(i*8+7)*4+1] | (src[i*7+6]  & 0x7f);
        }
        off += (((int) size/8) *7 );
/*余りの処理*/
        i = (((int) size/8) *8);
        if ( i < size){
            dst[i*4+1]     =  dst[i*4+1] | (src[off] >> 1 );
            add =1;
        }
        if ( i+1 < size){
            dst[(i+1)*4+1]   =dst[(i+1)*4+1] | (((src[off] << 6 ) | (src[off+1] >> 2 )) & 0x7f);
            add =2;
        }
        if ( i+2 < size){
            dst[(i+2)*4+1]   =dst[(i+2)*4+1] | (((src[off+1] << 5 ) | (src[off+2] >> 3 )) & 0x7f);
            add =3;
        }
        if ( i+3 < size){
            dst[(i+3)*4+1]   =dst[(i+3)*4+1] | (((src[off+2] << 4 ) | (src[off+3] >> 4 )) & 0x7f);
            add =4;
        }
        if ( i+4 < size){
            dst[(i+4)*4+1]   =dst[(i+4)*4+1] | (((src[off+3] << 3 ) | (src[off+4] >> 5 )) & 0x7f);
            add =5;
        }
        if ( i+5 < size){
            dst[(i+5)*4+1]   =dst[(i+5)*4+1] | (((src[off+4] << 2 ) | (src[off+5] >> 6 )) & 0x7f);
            add =6;
        }
        if ( i+6 < size){
            dst[(i+6)*4+1]   =dst[(i+6)*4+1] | (((src[off+5] << 1 ) | (src[off+6] >> 7 )) & 0x7f);
            add =7;
        }
        off += add;

    }else if (bit_size==16 && byte_size==8){
/*R8 仮数部1-3の処理*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<size;i++){
            dst[i*8 +byte_off -1]=dst[i*8 +byte_off -1] | (src[i*2] >> 4);
            dst[i*8 +byte_off]=(src[i*2] << 4) | (src[i*2+1] >> 4);
            dst[i*8 +byte_off +1]=(src[i*2+1] << 4);
        }
        off = size*2;
    }else if (bit_size==16 && byte_size==4){
/*R4 仮数部2の処理*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<size;i++){
            dst[i*4+2]=src[i*2];
            dst[i*4+3]=src[i*2+1];
        }
        off = size*2;
    }else if (bit_size==8){
/*R4 指数部の処理*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<size;i++){
            dst[i*4]=dst[i*4] | (src[i] >> 1) ; 
            dst[i*4 + 1]=(src[i] << 7);
        }
        off = size;
    }else if (bit_size==4){
/*R8 仮数部4の処理*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<size/2;i++){
            dst[i*8*2 + 7]=dst[i*8*2 + 7] | ((src[i] >> 4) & 0x0f); 
            dst[i*8*2 + 15]=dst[i*8*2 + 15] | (src[i] & 0x0f); 
        }
        off = size/2;
        if ((size % 2) ==1){
            dst[size*8 -1]=dst[size*8 -1] | ((src[off] >> 4) & 0x0f); 
            off ++;
        }
    }else if (bit_size==1){
/*R4,8 符号部の処理*/
        end_p = size / 8 ;
#ifdef OMP
#pragma omp parallel for private(i,j)
#endif
        for (i=0;i<end_p;i++){
            for (j=0;j<8;j++){
                dst[(i*8+j)*byte_size] = (( src[i] >> (7-j)) << 7);
            }
        }
/*余りの処理*/
        off =end_p;
        if (end_p*8 < size){
            for(j=0;j<8;j++){
                if (end_p*8+j < size){
                    dst[(end_p*8+j)*byte_size] = (( src[off] >> (7-j)) << 7);
                }
            }
            off++;
        }
    }
    return (off);
}
/*非圧縮を格納する*/
int nopack_store(unsigned char* buf, /*入力配列*/
                   unsigned char* out, /*出力配列*/
                   int size,/*入力配列の数*/
                   int byte_off,/*入力配列のオフセット*/
                   int bit_size,/*ターゲットのビット数*/
                   int byte_size/*入力配列のバイトサイズ*/
                   ){
    int i,end_p,add,off;
    if (bit_size==16 && byte_size == 8){
/* r8の仮数部1,2,3*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<size;i++){
             out[i*2]=(buf[i*8+byte_off-1] <<4 ) | (buf[i*8+byte_off] >>4 );
             out[i*2+1]=(buf[i*8+byte_off] <<4 ) | (buf[i*8+byte_off+1] >>4);
        }
        off = size*2;
    }else if (bit_size==16 && byte_size == 4){
/* r4の仮数部2*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<size;i++){
             out[i*2]=buf[i*4+2];
             out[i*2+1]=buf[i*4+3];
        }
        off = size*2;
    }else if (bit_size==8){
/* r4の指数部*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<size;i++){
             out[i]=(buf[i*4] << 1 ) | (buf[i*4+1] >> 7);
        }
        off = size;
    }else if (bit_size==11){
        end_p = (size / 8 );
/* r8の指数部*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<end_p;i++){
            out[i*11]  = (buf[i*64] << 1 ) | (buf[i*64+1] >> 7);
            out[i*11+1]= ((buf[i*64+1] << 1) & 0xe0 ) | ((buf[i*64+8] >> 2) & 0x1f);
            out[i*11+2]= (buf[i*64+8] << 6) | ((buf[i*64+9] >> 2) & 0x3c) | ((buf[i*64+16] >> 5) & 0x03);
            out[i*11+3]= (buf[i*64+16] << 3) | (buf[i*64+17] >> 5) ;
            out[i*11+4]= ((buf[i*64+17] << 3) & 0x80 ) | (buf[i*64+24] & 0x7f);
            out[i*11+5]= (buf[i*64+25]  & 0xf0 ) | ((buf[i*64+32]>>3) & 0x0f);
            out[i*11+6]= (buf[i*64+32] << 5) | ((buf[i*64+33] >> 3) & 0x1e) | ((buf[i*64+40] >> 6) & 0x01);
            out[i*11+7]= (buf[i*64+40] << 2 ) | (buf[i*64+41]>>6);
            out[i*11+8]= ((buf[i*64+41] << 2 ) & 0xc0) | ((buf[i*64+48]>>1) &0x3f);
            out[i*11+9]= (buf[i*64+48] << 7) | ((buf[i*64+49] >> 1) & 0x78) | ((buf[i*64+56] >> 4) & 0x07);
            out[i*11+10]=(buf[i*64+56] << 4 ) | ((buf[i*64+57]>>4) &0x0f);
        }
        off = end_p*11;
/*余りの処理*/
        i = end_p *8;
        add = 0;
        if ( i < size){
            out[off]  = (buf[i*8] << 1 ) | (buf[i*8+1] >> 7);
            out[off+1]= ((buf[i*8+1] << 1) & 0xe0 );
            add=2;
        }
        if ( (i+1) < size){
            out[off+1]= out[off+1] | ((buf[i*8+8] >> 2) & 0x1f);
            out[off+2]= (buf[i*8+8] << 6) | ((buf[i*8+9] >> 2) & 0x3c);
            add ++;
        }
        if ( (i+2) < size){
            out[off+2]= out[off+2] | ((buf[i*8+16] >> 5) & 0x03);
            out[off+3]= (buf[i*8+16] << 3) | (buf[i*8+17] >> 5) ;
            out[off+4]= ((buf[i*8+17] << 3) & 0x80 );
            add +=2;
        }
        if ( (i+3) < size){
            out[off+4]= out[off+4] | (buf[i*8+24] & 0x7f);
            out[off+5]= (buf[i*8+25]  & 0xf0 );
            add ++;
        }
        if ( (i+4) < size){
            out[off+5]= out[off+5] | ((buf[i*8+32]>>3) & 0x0f);
            out[off+6]= (buf[i*8+32] << 5) | ((buf[i*8+33] >> 3) & 0x1e);
            add ++;
        }
        if ( (i+5) < size){
            out[off+6]= out[off+6] | ((buf[i*8+40] >> 6) & 0x01);
            out[off+7]= (buf[i*8+40] << 2 ) | (buf[i*8+41]>>6);
            out[off+8]= ((buf[i*8+41] << 2 ) & 0xc0);
            add +=2;
        }
        if ( (i+6) < size){
            out[off+8]= out[off+8] | ((buf[i*8+48]>>1) &0x3f);
            out[off+9]= (buf[i*8+48] << 7) | ((buf[i*8+49] >> 1) & 0x78);
            add ++;
        }
        off +=add;
    }else if (bit_size==7){
        end_p = (size / 8 );
/* r4の仮数部1*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<end_p;i++){
            out[i*7]  = (buf[i*32+1] << 1 ) | ((buf[i*32+5] & 0x7f )>> 6);
            out[i*7+1]  = (buf[i*32+5] << 2 ) | ((buf[i*32+9] & 0x7f )>> 5);
            out[i*7+2]  = (buf[i*32+9] << 3 ) | ((buf[i*32+13] & 0x7f )>> 4);
            out[i*7+3]  = (buf[i*32+13] << 4 ) | ((buf[i*32+17] & 0x7f )>> 3);
            out[i*7+4]  = (buf[i*32+17] << 5 ) | ((buf[i*32+21] & 0x7f )>> 2);
            out[i*7+5]  = (buf[i*32+21] << 6 ) | ((buf[i*32+25] & 0x7f )>> 1);
            out[i*7+6]  = (buf[i*32+25] << 7 ) | (buf[i*32+29] & 0x7f );
        }
        off = end_p*7;
/*余りの処理*/
        i = end_p *8;
        add = 0;
        if ( i < size){
            out[off]  = (buf[i*4+1] << 1 );
            add=1;
        }
        if ( (i+1) < size){
            out[off]  = out[off] | ((buf[i*4+5] & 0x7f )>> 6);
            out[off+1]  = (buf[i*4+5] << 2 ) ;
            add =2;
        }
        if ( (i+2) < size){
            out[off+1]  = out[off+1] | ((buf[i*4+9] & 0x7f )>> 5);
            out[off+2]  = (buf[i*4+9] << 3 );
            add =3;
        }
        if ( (i+3) < size){
            out[off+2]  = out[off+2] | ((buf[i*4+13] & 0x7f )>> 4);
            out[off+3]  = (buf[i*4+13] << 4 );
            add =4;
        }
        if ( (i+4) < size){
            out[off+3]  = out[off+3] | ((buf[i*4+17] & 0x7f )>> 3);
            out[off+4]  = (buf[i*4+17] << 5 );
            add =5;
        }
        if ( (i+5) < size){
            out[off+4]  = out[off+4] | ((buf[i*4+21] & 0x7f )>> 2);
            out[off+5]  = (buf[i*4+21] << 6 );
            add =6;
        }
        if ( (i+6) < size){
            out[off+5]  = out[off+5] | ((buf[i*4+25] & 0x7f )>> 1);
            out[off+6]  = (buf[i*4+25] << 7 );
            add =7;
        }
        off +=add;

    }else if (bit_size==4){
/* r8の仮数部4*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
        for(i=0;i<size/2;i++){
            out[i]=buf[i*8*2+7]<<4 | (buf[i*8*2+15] & 0x0f );
        }
        off = size/2;
        if ((size % 2) ==1){
            out[off]=buf[(size-1)*8+7]<<4;
            off++;
        }
    }else if (bit_size==1){
/*符号部の処理*/
        end_p = ((int)(size / 8 )) * 8;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
        for (i=0;i<end_p;i=i+8){
            out[i/8] = (buf[i*byte_size] & 0x80) | ((buf[(i+1)*byte_size] & 0x80) >> 1) | ((buf[(i+2)*byte_size] & 0x80) >> 2) | ((buf[(i+3)*byte_size] & 0x80) >> 3) | ((buf[(i+4)*byte_size] & 0x80) >> 4) | ((buf[(i+5)*byte_size] & 0x80) >> 5) | ((buf[(i+6)*byte_size] & 0x80) >> 6) | ((buf[(i+7)*byte_size] & 0x80) >> 7);
        }
        off =(end_p/8);
        if (end_p < size ){
            out[off] = (buf[end_p*byte_size] & 0x80);
            for (i=1;i<8;i++){
                if ( (end_p + i) < size ){
                    out[off] = out[off] | ((buf[(end_p+i)*byte_size] & 0x80) >> i);
                }else{
                    break;
                }
            }
            off ++;
        }
    }
    return (off);
}
  
/*
    複合差分圧縮(の群圧縮の部分)を行う
*/
int encode_complex(unsigned  short* pack2, /*入力配列*/
                   unsigned int* raw, /*群の差分データ*/
                   int org_size,/*入力配列の数*/
                   int group_num,/*群の数*/
                   unsigned short* group_width,/*群の幅*/
                   unsigned short* group_ref,/*群の参照値*/
                   int* raw_pos/*群の差分データの群の先頭アドレス*/
                   ){

    int start_p,end_p,pos_bit,shift;
    int min_v,diff_b;
    int max_v;
    unsigned int diff_v;
    int i,j;
    int pos;
#ifdef OMP
#pragma omp parallel for private(i,j,pos_bit,pos,start_p,end_p,max_v,min_v,diff_b,diff_v,shift)
#endif
    for (i=0;i<group_num;i++){
        diff_b = 0;
        pos_bit=0;
        pos=0;
        start_p=i*group_size;
        end_p =org_size;
        if (i < (group_num -1 )){
            end_p=start_p + group_size;
        }
        min_v=pack2[start_p];
        max_v=pack2[start_p];
        for (j=(start_p+1);j<end_p;j++){
/*
            if (max_v < pack2[j]) {max_v = pack2[j];continue;}
            if (min_v > pack2[j]) {min_v = pack2[j];}
            */
/*やっていることは最大/最小値を求める */
            max_v < pack2[j] && (max_v = pack2[j]),(min_v > pack2[j] && (min_v = pack2[j]),min_v);
        }
        diff_v= (max_v - min_v);
#ifdef DEBUG
        if (debug > 1 ){
            printf("group_no =%d Diff =%u Max=%d Min=%d start_p=%d end_p=%d\n",i,diff_v,max_v,min_v,start_p,end_p);
        }
#endif
/*        while (diff_v != 0){
            diff_b ++;
            diff_v = diff_v >> 1;
        }*/
/*高速化のため2分木にする*/
        if (diff_v < 128){
            if(diff_v < 8){
                if (diff_v <2){
                    if (diff_v < 1){ diff_b=0;
                    }else{ diff_b=1;}
                }else{
                    if (diff_v < 4){ diff_b=2;
                    }else{ diff_b=3;}
                }
            }else{
                if (diff_v <32){
                    if (diff_v < 16){ diff_b=4;
                    }else{ diff_b=5;}
                }else{
                    if (diff_v < 64){ diff_b=6;
                    }else{ diff_b=7;}
                }
            }
            group_width[i]=diff_b;
        }else{
            if(diff_v < 2048){
                if (diff_v <512){
                    if (diff_v < 256){ diff_b=8;
                    }else{ diff_b=9;}
                }else{
                    if (diff_v < 1024){ diff_b=10;
                    }else{ diff_b=11;}
                }
                group_width[i]=diff_b;
            }else{
                if (diff_v <8192){
                    if (diff_v < 4096){ diff_b=12;
                    }else{ diff_b=13;}
                    group_width[i]=diff_b;
                }else{
                    if (diff_v < 16384){ 
                        diff_b=14;
                        group_width[i]=diff_b;
                    }else{ 
                        group_width[i]=15;
                        diff_b=16;
                    }
                }
            }
        }
/*群の幅は4ビットに抑えるため15と16は15で格納*/
/*        if (diff_b < 15){
            group_width[i]=diff_b;
        }else{
            group_width[i]=15;
            diff_b =16;
        }*/
        group_ref[i]=min_v;
        if (diff_b > 0) {
           pos=start_p;
           for (j=start_p;j<end_p;j++){
                diff_v=pack2[j] - min_v;
                if ((shift=(pos_bit+diff_b)) > 32){
                    pos_bit = shift - 32;
                    raw[pos]=raw[pos] | (diff_v >> pos_bit);
                    pos ++;
                    raw[pos]= (diff_v << (64 - shift));
                } else if (pos_bit!=0){
                    raw[pos]=raw[pos] | (diff_v << (32 - shift));
                    pos_bit += diff_b;
                    if (pos_bit == 32 ){
                        pos += 1;
                        pos_bit =0;
                    }
                } else{
                    raw[pos]=diff_v << (32 - shift);
                    pos_bit += diff_b;
                }
            }
            raw_pos[i]=pos;
            if (pos_bit !=0){
                raw_pos[i] ++;
            }
        }else{
            raw_pos[i]=start_p;
        }
    }
    return (pos);
}
/*
  1bitのランレングス圧縮を行う

*/
int encode_runlength(unsigned char * buf,/*入力配列*/
                     unsigned char * out,/*出力配列*/
                     int org_size,/*入力配列の大きさ*/
                     int org_byte_size,/*入力配列1要素の大きさ(単位Byte)*/
                     int org_byte_off,/*圧縮する1ビットデータの先頭からのoffsetバイト数*/
                     int org_bit_off/*圧縮する1ビットデータのorg_byte_offからのoffsetビット数*/
                     ){
    int i;
/*    unsigned char sign,old_sign;*/
    unsigned int sign,old_sign;
    int sign_count=-1,sign_count2;
    int sign_count3;
    int bit_pos=0;
    int off=0;
    const unsigned char mask2[8]={0x80,0x40,0x20,0x10,0x08,0x04,0x02,0x01};
/*    unsigned char v[3];*/
    unsigned int vv;

    old_sign = (buf[org_byte_off] & mask2[org_bit_off]);
/*    v[0] = old_sign >> (7-org_bit_off );*/
    vv = old_sign >> (7-org_bit_off );
    bit_pos=1;
    for (i=org_byte_size+org_byte_off;i<org_size*org_byte_size+org_byte_off;i+=org_byte_size){
        sign = (buf[i] & mask2[org_bit_off]);
        if (old_sign == sign){
            sign_count ++;
        }else{
/*値が異なったら書き出す*/
            old_sign = sign;
/* sign_counthは実際の数より-2した値を格納する*/
            if (sign_count != 0){
                while(sign_count >0){
/*下位ビットから3進数で格納していく*/
/*                    sign_count2 = sign_count % 3;
                    sign_count = sign_count /3;*/
                    sign_count3 = sign_count /3;
                    sign_count2 = sign_count - (sign_count3 * 3);
                    sign_count = sign_count3;
                    if(bit_pos != 3){
/*                        v[bit_pos] = sign_count2;*/
                        vv = (vv << 2) + sign_count2;
                        bit_pos ++;
                    }else{
                        out[off] = (vv << 2) + sign_count2;
                        off ++;
                        bit_pos=0;
                    }
                }
            }else{
                if(bit_pos != 3){
                    vv = (vv <<2);
                    bit_pos ++;
                }else{
                    out[off] = (vv << 2);
                    off ++;
                    bit_pos=0;
                }
            }
            if(bit_pos != 3){
                vv = (vv << 2) + 3;
                bit_pos ++;
            }else{
                out[off] = (vv << 2) + 3;
                off ++;
                bit_pos=0;
            }
            sign_count = -1;
        }
    }
/*余りの処理*/
    if (sign_count != 0){
        while(sign_count >0){
/*            sign_count2 = sign_count % 3;
            sign_count = sign_count /3; */
            sign_count3 = sign_count /3;
            sign_count2 = sign_count - (sign_count3 * 3);
            sign_count = sign_count3;
            if(bit_pos != 3){
                vv = (vv << 2) + sign_count2;
                bit_pos ++;
            }else{
                out[off] = (vv << 2) + sign_count2;
                off ++;
                bit_pos=0;
            }
        }
    } else{
        if(bit_pos != 3){
            vv = (vv <<2);
            bit_pos ++;
        }else{
            out[off] = (vv <<2);
            off ++;
            bit_pos=0;
        }
    }
    if(bit_pos == 3){
        out[off] = (vv << 2) + 3;
    }else if(bit_pos == 2){
        out[off] = (((vv & 0x0f) << 2) +3) << 2;
    }else if(bit_pos == 1){
        out[off] = (((vv & 0x03) << 2) +3) << 4;
    }else{
        out[off] = 0xc0;
    }
    off ++;
    return(off);
}
/*
  r4デコード本体
*/
int decode_r4(const unsigned char *src, /*Packingされたデータ*/
              unsigned char *dst, /*解凍したものを詰め込む配列*/
              int little_endian /* 0: big endian 1:little_endian*/
              ){
    int group_num,org_size;
    int i,j,k;
    unsigned short *group_ref;
    unsigned short *group_width;
    int *group_adr;
    unsigned char flag_chk;
    unsigned short *unpack;
    int *pack;
    int off=0;/* 元データの読み込み開始位置*/
    int off2;
    unsigned char flag1;/* 9バイト目に格納されている*/
    unsigned char flag2;/* 10バイト目に格納されている*/

    int pack_offset;
    int pack_offset2;
    int pack_size;
    int rc;
    int first_adr[16];
    int bit_adr[16];
    int rcs[16];
    unsigned char *runlength_data;
    int threads=1;
    int e_top;/*指数部の先頭アドレス*/
    int f_top1,f_top2;/*仮数部の先頭アドレス*/
    int end_p;
    int org_size2;
    int com_alloc_flag=0;
    int run_alloc_flag=0;
/*    const unsigned int mask2[17]={0,0x00000001,0x00000003,0x00000007,0x0000000f,0x0000001f,0x0000003f,0x0000007f,0x000000ff,
                                    0x000001ff,0x000003ff,0x000007ff,0x00000fff,0x00001fff,0x00003fff,0x00007fff,0x0000ffff};*/

#ifdef OMP
    threads=omp_get_max_threads();
#endif
    org_size=PEEK_N_UI4(src);
    off +=4;
/*データ長は単に読んでおくだけ*/
    pack_size=PEEK_N_UI4(src+off);
    off +=4;
    flag1 = src[off];
    off ++;
    flag2 = src[off];
    off ++;
/*指数部の先頭アドレス読む*/
    e_top=PEEK_N_UI4(src+off);
    off += 4;
/*(現在利用していなけど)仮数部の先頭アドレス読む*/
    f_top1=PEEK_N_UI4(src+off);
    off += 4;
    f_top2=PEEK_N_UI4(src+off);
    off += 4;
#ifdef DEBUG
    if (debug > 0){
        printf("Start Decode Flag1=%d flag2=%d size=%d endian =%d threads=%d\n",flag1,flag2,org_size,little_endian,threads);fflush(stdout);
    }
#endif
    for(i=0;i<9;i++){
        rcs[i] =0;
    }
/*符号ビットの圧縮の有無*/
    if ((flag1 & 0x80) == 0x80 ){
        if ((runlength_data = (unsigned char*)malloc(org_size*16)) == NULL){
            printf("malloc error(runlength_data) size=%d\n",org_size*16);
            exit(1);
        }
        run_alloc_flag=1;
/* 符号はランレングス圧縮されていた*/
        if ((flag1 & 0x20)== 0x20) {
/*指数もランレングス圧縮されている*/
            first_adr[0] = e_top+32;
            for(i=1;i<8;i++){
                first_adr[i] =first_adr[i-1] + PEEK_N_UI4(src+e_top+(i-1)*4);
            }
            first_adr[8] = off;
            bit_adr[8]=0;
            for(i=0;i<7;i++){
                bit_adr[i]=i+1;
            }
            bit_adr[7]=0;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
            for(i=0;i<9;i++){
                rcs[i]=decode_runlength((unsigned char *)src+first_adr[i],runlength_data+org_size*i,org_size,1,0,bit_adr[i]);
            }
/*ランレングス圧縮解凍データを符号,指数一緒に出力配列に詰め込む*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
            for(i=0;i<org_size;i++){
                dst[i*4] = (runlength_data[org_size*8+i] | runlength_data[i] | runlength_data[i+org_size] | runlength_data[i+org_size*2] | runlength_data[i+org_size*3] | runlength_data[i+org_size*4] | runlength_data[i+org_size*5] |runlength_data[i+org_size*6] ); 
                dst[i*4+1] = runlength_data[i+org_size*7] ;
            }
            off +=rcs[8];
        }else{
/*指数部はランレングス圧縮されていないので、符号部だけ解凍し直接出力配列に書き込む*/
            rc=decode_runlength((unsigned char *)src+off,dst,org_size,4,0,0);
            off +=rc;
        }
    }else{
/*残念ながら圧縮効率が悪いので単に詰めていたものを解凍する*/
#ifdef DEBUG
        if (debug > 0){
            printf("Sign is No packing\n");
        }
#endif
        off = off + nopack_restore(src+off, dst, org_size, 0, 1,4);
    }
/* 複合差分圧縮解凍本体*/
    group_num=(org_size-1)/group_size +1;
    for (k=0;k<3;k++){
        org_size2 = org_size;
        if (k==0){ flag_chk= ((flag1>>4) & 0x03);
        }else if(k==1){ flag_chk= ((flag1>>2) & 0x03); 
        }else { flag_chk= (flag1 & 0x03);
        }
#ifdef DEBUG
        if (debug > 0){
            printf("UnPacking Part =%d flag_chk=%d offset=%d \n",k,flag_chk,off);fflush(stdout);
        }
#endif
        if (flag_chk == 1 ){
/*複合差分圧縮だ*/
            if (com_alloc_flag==0){
                if ((group_width = (unsigned short*)malloc(2 * (group_num+1))) == NULL){
                    printf("malloc error(group_width) size=%d\n",2 * (group_num+1));
                    exit(1);
                }
                if ((group_ref = (unsigned short*)malloc(2 * (group_num+1))) == NULL){
                    printf("malloc error(group_ref) size=%d\n",2 * (group_num+1));
                    exit(1);
                }
                if ((group_adr = (int *)malloc(4 * group_num)) == NULL){
                    printf("malloc error(group_adr) size=%d\n",4 * group_num);
                    exit(1);
                }
                if ((pack = (int *)malloc(4 * (org_size+3))) == NULL){
                    printf("malloc error(pack) size=%d\n",4 * (org_size+3));
                    exit(1);
                }
                if ((unpack = (unsigned short*)malloc(2 * org_size)) == NULL){
                    printf("malloc error(unpack) size=%d\n",2 * org_size);
                    exit(1);
                }
                com_alloc_flag=1;
            }
#ifdef DEBUG
            if (debug > 1 ){
                printf("group_ref offset=%d\n",off);
            }
#endif
            if (k==0){
                pack_offset = pack_offset8;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<group_num;i++){
                    group_ref[i] = src[off+i];
                }
                off += group_num;
            }else if (k==1){
                pack_offset = pack_offset7;
                end_p = group_num/8;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<end_p;i++){
                    group_ref[i*8]=(src[off+i*7]>>1);
                    group_ref[i*8+1]=((src[off+i*7] & 0x01)<<6)|(src[off+i*7+1] >>2);
                    group_ref[i*8+2]=((src[off+i*7+1] & 0x03) <<5)|(src[off+i*7+2] >>3);
                    group_ref[i*8+3]=((src[off+i*7+2] & 0x07) <<4)|(src[off+i*7+3] >>4);
                    group_ref[i*8+4]=((src[off+i*7+3] & 0x0f) <<3)|(src[off+i*7+4] >>5);
                    group_ref[i*8+5]=((src[off+i*7+4] & 0x1f) <<2)|(src[off+i*7+5] >>6);
                    group_ref[i*8+6]=((src[off+i*7+5] & 0x3f) <<1)|(src[off+i*7+6] >>7);
                    group_ref[i*8+7]=(src[off+i*7+6] & 0x7f);
                }
                off +=end_p*7;
/*余りの処理*/
                if (end_p*8 < group_num){
                    group_ref[end_p*8]=(src[off]>>1);
                    off ++;
                }
                if ((end_p*8+1) < group_num){
                    group_ref[end_p*8+1]=((src[off-1] & 0x01)<<6)|(src[off] >>2);
                    off ++;
                }
                if ((end_p*8+2) < group_num){
                    group_ref[end_p*8+2]=((src[off-1] & 0x03) <<5)|(src[off] >>3);
                    off ++;
                }
                if ((end_p*8+3) < group_num){
                    group_ref[end_p*8+3]=((src[off-1] & 0x07) <<4)|(src[off] >>4);
                    off ++;
                }
                if ((end_p*8+4) < group_num){
                    group_ref[end_p*8+4]=((src[off-1] & 0x0f) <<3)|(src[off] >>5);
                    off ++;
                }
                if ((end_p*8+5) < group_num){
                    group_ref[end_p*8+5]=((src[off-1] & 0x1f) <<2)|(src[off] >>6);
                    off ++;
                }
                if ((end_p*8+6) < group_num){
                    group_ref[end_p*8+6]=((src[off-1] & 0x3f) <<1)|(src[off] >>7);
                    off ++;
                }
            }else{
                pack_offset = pack_offset16;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<group_num;i++){
                    group_ref[i] = PEEK_N_UI2(src+off+i*2);
                }
                off += (group_num*2);
            }
#ifdef DEBUG
            if (debug > 1 ){
                printf("group_w offset=%d\n",off);
            }
#endif
#ifdef OMP
#pragma omp parallel for private(i)
#endif
            for(i=0;i<((group_num-1)/2+1);i++){
                group_width[i*2]=src[off+i] >>4;
                group_width[i*2+1]=src[off+i] & 0x0F;
/*群の幅は4ビットに抑えるため15と16は15で格納されている*/
                if (group_width[i*2] == 15){
                    group_width[i*2] = 16;
                }
                if (group_width[i*2+1] == 15){
                    group_width[i*2+1] = 16;
                }

            }
            off += ((group_num-1)/2 + 1);
/*
    並列化のために各グループの先頭アドレスを求める
*/
            for(i=0;i<(group_num-1);i++){
                group_adr[i]=off;
                off += (group_width[i]*group_size/8);
            }
            group_adr[group_num-1]=off;
/*最後は半端があるので真面目に計算*/
            if (group_width[group_num-1] > 0){
                off +=  ((int)(((org_size2 - (group_num-1)*group_size)*group_width[group_num-1] - 1)/32)) * 4 + 4;
            }
#ifdef DEBUG
            if (debug > 1 ){
                printf("raw_w offset=%d\n",off);
            }
#endif
/*各差分値(pack)を求める*/
/* 以下のループは並列化可能*/
#ifdef OMP
#pragma omp parallel for private(i,j,end_p)
#endif
            for(i=0;i<group_num;i++){
                int pos=0;
                int pos_bit=0;
                unsigned int diff_v,diff_v1;
                int start_p;

#ifdef DEBUG
                if (debug > 1 ){
                    printf("group_no= %d group_width=%d group_ref=%d\n",i,group_width[i],group_ref[i]);
                }
#endif
                start_p=i*group_size;
                end_p=org_size2;
                if (i < (group_num -1 )){
                    end_p=start_p + group_size;
                }
                diff_v = PEEK_N_UI4(src+group_adr[i]);
                if (group_width[i] > 0){
                    for(j=start_p;j<end_p;j++){
                        if ((pos_bit+group_width[i]) > 32){
                            diff_v1=(( diff_v <<  pos_bit) >> (32 - group_width[i])) ;
                            pos += 4;
                            diff_v = PEEK_N_UI4(src+group_adr[i]+pos);
                            pack[j]=(diff_v1 | ( diff_v >> (64 - (pos_bit + group_width[i]))) ) +group_ref[i];
                            pos_bit = pos_bit+group_width[i] - 32;
                        }else if ((pos_bit+group_width[i]) != 32) {
                            pack[j]=(( diff_v <<  pos_bit) >> (32 - group_width[i])) +group_ref[i];
                            pos_bit = pos_bit+group_width[i];
                        } else{
                            pack[j]=(( diff_v <<  pos_bit) >> (32 - group_width[i]))  +group_ref[i];
/*ここがマスクに変えられるが遅くなる*/
/*                            pack[j]=(diff_v & mask2[group_width[i]])  +group_ref[i];*/
                            pos += 4;
                            diff_v = PEEK_N_UI4(src+group_adr[i]+pos);
                            pos_bit = 0;
                        }
                    }
                }else{
                    for(j=start_p;j<end_p;j++){
                        pack[j]=group_ref[i];
                    }
                }
            }
/*指数部は11bitなので特別処理をする*/
/*
            if (k==0){
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<org_size;i++){
                    if (pack[i] >=2048){
                        pack[i] -=2048; }
                }
            }
*/
            pack_offset2 = pack_offset * 2;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
            for(i=0;i<org_size2;i++){
                pack[i] -= pack_offset;
                if ( pack[i] < 0 ){
                    pack[i] += pack_offset2;
                }
            }
            unpack[0]=pack[0];
#ifdef SD
            unpack[1]=pack[1];
            for(i=2;i<org_size2;i++){
                unpack[i]=( 2*unpack[i-1] ) - unpack[i-2] + pack[i];
            }
#else
            for(i=1;i<org_size2;i++){
                unpack[i]= unpack[i-1] + pack[i];
            }
#endif
/* データを格納する*/
            if (k==0){
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<org_size;i++){
                    dst[i*4] = dst[i*4] | ((unpack[i] >> 1) & 0x7f);
                    dst[i*4+1] = unpack[i] << 7;
                }
            }else if(k==1 ){ 
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<org_size;i++){
                    dst[i*4+1] = dst[i*4+1] | (unpack[i] & 0x7f);
                }
            }else if(k==2 ){ 
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<org_size;i++){
                    dst[i*4+2] = unpack[i]>>8;
                    dst[i*4+3] = unpack[i];
                }
            }
        }else if(flag_chk==2){
/*ランレングス圧縮を解凍する*/
            if (run_alloc_flag==0){
                if ((runlength_data = (unsigned char*)malloc(org_size*16)) == NULL){
                    printf("malloc error(runlength_data) size=%d\n",org_size*16);
                    exit(1);
                }
                run_alloc_flag=1;
            }
            if (k==0){
                if (rcs[0]==0){
                    for(i=0;i<7;i++){
                        bit_adr[i]=i+1;
                    }
                    bit_adr[7]=0;
                    off2= off;
                    first_adr[0] = off + 32;
                    for(i=1;i<8;i++){
                        first_adr[i] =first_adr[i-1] + PEEK_N_UI4(src+off+(i-1)*4);
                    }
                    for(j=0;j<8;j++){
                        off += PEEK_N_UI4(src+off2+j*4);
                    }
                    off += 32;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                    for(i=0;i<8;i++){
#ifdef DEBUG
                        if (debug > 0){
                            printf("RunLength Decode(%d) Part=%d packing adr=%d %d\n",i,k,first_adr[i],src[first_adr[i]]);
                        }
#endif
                        rcs[i]=decode_runlength((unsigned char *)src+first_adr[i],runlength_data+org_size*i,org_size,1,0,bit_adr[i]);
                    }
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                    for(i=0;i<org_size;i++){
                        dst[i*4] = (dst[i*4] | runlength_data[i] | runlength_data[i+org_size] | runlength_data[i+org_size*2] | runlength_data[i+org_size*3] | runlength_data[i+org_size*4] | runlength_data[i+org_size*5] |runlength_data[i+org_size*6] ); 
                        dst[i*4+1] = runlength_data[i+org_size*7]; 
                    }
                }else{
/*既にランレングス解凍は符号部と一緒にやっている*/
                    off2= off;
                    for(j=0;j<8;j++){
                        off += PEEK_N_UI4(src+off2+j*4);
                    }
                    off += 32;
                }
            }else if (k==1){
                for(i=0;i<7;i++){
                    bit_adr[i]=i+1;
                }
                off2= off;
                first_adr[0] = off + 28;
                for(i=1;i<7;i++){
                    first_adr[i] =first_adr[i-1] + PEEK_N_UI4(src+off+(i-1)*4);
                }
                for(j=0;j<7;j++){
                    off += PEEK_N_UI4(src+off2+j*4);
                }
                off += 28;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<7;i++){
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Decode(%d) Part=%d packing adr=%d %u\n",i,k,first_adr[i],src[first_adr[i]]);fflush(stdout);
                    }
#endif
                    rcs[i]=decode_runlength((unsigned char *)src+first_adr[i],runlength_data+org_size*i,org_size,1,0,bit_adr[i]);
                }
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<org_size;i++){
                    dst[i*4+1] = (dst[i*4+1] | runlength_data[i] | runlength_data[i+org_size] | runlength_data[i+org_size*2] | runlength_data[i+org_size*3] | runlength_data[i+org_size*4] | runlength_data[i+org_size*5] |runlength_data[i+org_size*6] ); 
                }
            }else{
                for(i=0;i<8;i++){
                    bit_adr[i]=i;
                    bit_adr[i+8]=i;
                }
                off2= off;
                first_adr[0] = off + 64;
                for(i=1;i<16;i++){
                    first_adr[i] =first_adr[i-1] + PEEK_N_UI4(src+off+(i-1)*4);
                }
                for(j=0;j<16;j++){
                    off += PEEK_N_UI4(src+off2+j*4);
                }
                off += 64;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<16;i++){
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Decode(%d) Part=%d packing first_adr=%d\n",i,k,first_adr[i]);
                    }
#endif
                    rcs[i]=decode_runlength((unsigned char *)src+first_adr[i],runlength_data+org_size*i,org_size,1,0,bit_adr[i]);
                }
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<org_size;i++){
                    dst[i*4+2] = (runlength_data[i] | runlength_data[i+org_size] | runlength_data[i+org_size*2] | runlength_data[i+org_size*3] | runlength_data[i+org_size*4] | runlength_data[i+org_size*5] |runlength_data[i+org_size*6] | runlength_data[i+org_size*7]); 
                    dst[i*4+3] = (runlength_data[i+org_size*8] | runlength_data[i+org_size*9] | runlength_data[i+org_size*10] | runlength_data[i+org_size*11] | runlength_data[i+org_size*12] | runlength_data[i+org_size*13] |runlength_data[i+org_size*14] | runlength_data[i+org_size*15]); 
                }
            }
        }else{
/*圧縮が効かなかった悲しいケース*/
#ifdef DEBUG
            if (debug > 0){
                printf("UnPacking Part=%d No packing offset=%d\n",k,off);
            }
#endif
            if (k==0){
                off +=nopack_restore(src+off, dst, org_size, 0, 8,4);
            }else if(k==1){
                off +=nopack_restore(src+off, dst, org_size, 0, 7,4);
            }else{
                off +=nopack_restore(src+off, dst, org_size, 0, 16,4);
            }
        }
    }
    if (little_endian == 1){
        unsigned char swap;
#ifdef OMP
#pragma omp parallel for private(i,j,swap)
#endif
        for(i=0;i<org_size*4;i+=4){
            for(j=0;j<2;j++){
                swap = dst[i+j];
                dst[i+j] = dst[i+3-j];
                dst[i+3-j] = swap;
            }
        }
    }
#ifdef DEBUG
    if (debug > 0){
        if (pack_size != off){
            printf("X Unpack Size Err store=%d off =%d\n",pack_size,off);
            exit(1);
        }
    }
#endif
    if (run_alloc_flag==1){
        free(runlength_data);
    }
    if (com_alloc_flag==1){
        free(group_width);
        free(group_ref);
        free(group_adr);
        free(pack);
        free(unpack);
    }
    return (org_size);
}
/*
  r8デコード本体
*/
int decode_r8(const unsigned char *src, /*Packingされたデータ*/
              unsigned char *dst, /*解凍したものを詰め込む配列*/
              int little_endian /* 0: big endian 1:little_endian*/
              ) {
    int group_num,org_size;
    int i,j,k;
    unsigned short *group_ref;
    unsigned short *group_width;
    int *group_adr;
    unsigned char flag_chk;
    unsigned short *unpack;
    int *pack;
    unsigned char buf2[2];
    int off=0;/* 元データの読み込み開始位置*/
    int off2;
    unsigned char flag1;/* 9バイト目に格納されている*/
    unsigned char flag2;/* 10バイト目に格納されている*/

    int pack_offset;
    int pack_offset2;
    int pack_size;
    int rc;
    int first_adr[16];
    int bit_adr[16];
    int rcs[16];
    unsigned char *runlength_data;
    int threads=1;
    int e_top;/*指数部の先頭アドレス*/
    int f_top1,f_top2,f_top3,f_top4;/*仮数部の先頭アドレス*/
    int end_p;
    int org_size2;
    int com_alloc_flag=0;
    int run_alloc_flag=0;
    int add;
/*    const unsigned int mask2[17]={0,0x00000001,0x00000003,0x00000007,0x0000000f,0x0000001f,0x0000003f,0x0000007f,0x000000ff,
                                    0x000001ff,0x000003ff,0x000007ff,0x00000fff,0x00001fff,0x00003fff,0x00007fff,0x0000ffff};*/

#ifdef OMP
    threads=omp_get_max_threads();
#endif
    org_size=PEEK_N_UI4(src);
    off +=4;
/*データ長は単に読んでおくだけ*/
    pack_size=PEEK_N_UI4(src+off);
    off +=4;
    flag1 = src[off];
    off ++;
    flag2 = src[off];
    off ++;
/*指数部の先頭アドレス読む*/
    e_top=PEEK_N_UI4(src+off);
    off += 4;
/*(現在利用していなけど)仮数部の先頭アドレス読む*/
    f_top1=PEEK_N_UI4(src+off);
    off += 4;
    f_top2=PEEK_N_UI4(src+off);
    off += 4;
    f_top3=PEEK_N_UI4(src+off);
    off += 4;
    f_top4=PEEK_N_UI4(src+off);
    off += 4;
#ifdef DEBUG
    if (debug > 0){
        printf("Start Decode Flag1=%d flag2=%d size=%d endian =%d threads=%d\n",flag1,flag2,org_size,little_endian,threads);fflush(stdout);
    }
#endif
    for(i=0;i<12;i++){
        rcs[i] =0;
    }
/*符号ビットの圧縮の有無*/
    if ((flag1 & 0x80) == 0x80 ){
        if ((runlength_data = (unsigned char*)malloc(org_size*16)) == NULL){
            printf("malloc error(runlength_data) size=%d\n",org_size*16);
            exit(1);
        }
        run_alloc_flag=1;
/* 符号はランレングス圧縮されていた*/
        if ((flag1 & 0x20)== 0x20) {
/*指数もランレングス圧縮されている*/
            first_adr[0] = e_top+44;
            for(i=1;i<11;i++){
                first_adr[i] =first_adr[i-1] + PEEK_N_UI4(src+e_top+(i-1)*4);
            }
            first_adr[11] = off;
            bit_adr[11]=0;
            for(i=0;i<7;i++){
                bit_adr[i]=i+1;
            }
            for(i=7;i<11;i++){
                bit_adr[i]=i-7;
            }
#ifdef OMP
#pragma omp parallel for private(i)
#endif
            for(i=0;i<12;i++){
                rcs[i]=decode_runlength((unsigned char *)src+first_adr[i],runlength_data+org_size*i,org_size,1,0,bit_adr[i]);
            }
/*ランレングス圧縮解凍データを符号,指数一緒に出力配列に詰め込む*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
            for(i=0;i<org_size;i++){
                dst[i*8] = (runlength_data[org_size*11+i] | runlength_data[i] | runlength_data[i+org_size] | runlength_data[i+org_size*2] | runlength_data[i+org_size*3] | runlength_data[i+org_size*4] | runlength_data[i+org_size*5] |runlength_data[i+org_size*6] ); 
                dst[i*8+1] = (runlength_data[i+org_size*7] | runlength_data[i+org_size*8] | runlength_data[i+org_size*9] | runlength_data[i+org_size*10]); 
            }
            off +=rcs[11];
        }else{
/*指数部はランレングス圧縮されていないので、符号部だけ解凍し直接出力配列に書き込む*/
            rc=decode_runlength((unsigned char *)src+off,dst,org_size,8,0,0);
            off +=rc;
        }
    }else{
/*残念ながら圧縮効率が悪いので単に詰めていたものを解凍する*/
#ifdef DEBUG
        if (debug > 0){
            printf("Sign is No packing\n");
        }
#endif
        off = off + nopack_restore(src+off, dst, org_size, 0, 1,8);
    }
/* 複合差分圧縮解凍本体*/
    group_num=(org_size-1)/group_size +1;
    for (k=0;k<5;k++){
        org_size2 = org_size;
        if (k==0){ flag_chk= ((flag1>>4) & 0x03);
        }else if(k==1){ flag_chk= ((flag1>>2) & 0x03); 
        }else if(k==2){ flag_chk= (flag1 & 0x03);
        }else if(k==3){ flag_chk= ((flag2>>6) & 0x03);
        }else{ flag_chk= ((flag2>>4) & 0x03);
        }
#ifdef DEBUG
        if (debug > 0){
            printf("UnPacking Part =%d flag_chk=%d offset=%d \n",k,flag_chk,off);fflush(stdout);
        }
#endif
        if (flag_chk == 1 ){
/*複合差分圧縮だ*/
            if (com_alloc_flag==0){
                if ((group_width = (unsigned short*)malloc(2 * (group_num+1))) == NULL){
                    printf("malloc error(group_width) size=%d\n",2 * (group_num+1));
                    exit(1);
                }
                if ((group_ref = (unsigned short*)malloc(2 * (group_num+1))) == NULL){
                    printf("malloc error(group_ref) size=%d\n",2 * (group_num+1));
                    exit(1);
                }
                if ((group_adr = (int *)malloc(4 * group_num)) == NULL){
                    printf("malloc error(group_adr) size=%d\n",4 * group_num);
                    exit(1);
                }
                if ((pack = (int *)malloc(4 * (org_size+3))) == NULL){
                    printf("malloc error(pack) size=%d\n",4 * (org_size+3));
                    exit(1);
                }
                if ((unpack = (unsigned short*)malloc(2 * org_size)) == NULL){
                    printf("malloc error(unpack) size=%d\n",2 * org_size);
                    exit(1);
                }
                com_alloc_flag=1;
            }
#ifdef DEBUG
            if (debug > 1 ){
                printf("group_ref offset=%d\n",off);
            }
#endif
            if (k==4){
                group_num=(org_size-1)/group_size/4 +1;
                org_size2 = (org_size - 1)/4 + 1;
            }
            if (k==0){
                pack_offset = pack_offset11;
                end_p = group_num/8;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<end_p;i++){
                    group_ref[i*8]=(src[off+i*11]<<3)|(src[off+i*11+1] >>5);
                    group_ref[i*8+1]=(src[off+i*11+1] <<6)|(src[off+i*11+2] >>2);
                    group_ref[i*8+2]=(src[off+i*11+2]<<9)|(src[off+i*11+3] <<1) | (src[off+i*11+4] >>7);
                    group_ref[i*8+3]=(src[off+i*11+4]<<4)|(src[off+i*11+5] >>4);
                    group_ref[i*8+4]=(src[off+i*11+5]<<7)|(src[off+i*11+6] >>1);
                    group_ref[i*8+5]=(src[off+i*11+6]<<10)|(src[off+i*11+7] <<2) | (src[off+i*11+8] >>6);
                    group_ref[i*8+6]=(src[off+i*11+8]<<5)|(src[off+i*11+9] >>3);
                    group_ref[i*8+7]=(src[off+i*11+9]<<8)|(src[off+i*11+10]);
                }
                off +=end_p*11;
                add=0;
/*余りの処理*/
                if (end_p*8 < group_num){
                    group_ref[end_p*8]=(src[off]<<3)|(src[off+1] >>5);
                    add=2;
                }
                if ((end_p*8+1) < group_num){
                    group_ref[end_p*8+1]=(src[off+1] <<6)|(src[off+2] >>2);
                    add=3;
                }
                if ((end_p*8+2) < group_num){
                    group_ref[end_p*8+2]=(src[off+2]<<9)|(src[off+3] <<1) | (src[off+4] >>7);
                    add=5;
                }
                if ((end_p*8+3) < group_num){
                    group_ref[end_p*8+3]=(src[off+4]<<4)|(src[off+5] >>4);
                    add=6;
                }
                if ((end_p*8+4) < group_num){
                    group_ref[end_p*8+4]=(src[off+5]<<7)|(src[off+6] >>1);
                    add=7;
                }
                if ((end_p*8+5) < group_num){
                    group_ref[end_p*8+5]=(src[off+6]<<10)|(src[off+7] <<2) | (src[off+8] >>6);
                    add=9;
                }
                if ((end_p*8+6) < group_num){
                    group_ref[end_p*8+6]=(src[off+8]<<5)|(src[off+9] >>3);
                    add=10;
                }
                off += add;
            }else{
                pack_offset = pack_offset16;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<group_num;i++){
                    group_ref[i] = PEEK_N_UI2(src+off+i*2);
                }
                off += (group_num*2);
            }
#ifdef DEBUG
            if (debug > 1 ){
                printf("group_w offset=%d\n",off);
            }
#endif
#ifdef OMP
#pragma omp parallel for private(i)
#endif
            for(i=0;i<((group_num-1)/2+1);i++){
                group_width[i*2]=src[off+i] >>4;
                group_width[i*2+1]=src[off+i] & 0x0F;
/*群の幅は4ビットに抑えるため15と16は15で格納されている*/
                if (group_width[i*2] == 15){
                    group_width[i*2] = 16;
                }
                if (group_width[i*2+1] == 15){
                    group_width[i*2+1] = 16;
                }

            }
            off += ((group_num-1)/2 + 1);
/*
    並列化のために各グループの先頭アドレスを求める
*/
            for(i=0;i<(group_num-1);i++){
                group_adr[i]=off;
                off += (group_width[i]*group_size/8);
            }
            group_adr[group_num-1]=off;
/*最後は半端があるので真面目に計算*/
            if (group_width[group_num-1] > 0){
                off +=  ((int)(((org_size2 - (group_num-1)*group_size)*group_width[group_num-1] - 1)/32)) * 4 + 4;
            }
#ifdef DEBUG
            if (debug > 1 ){
                printf("raw_w offset=%d\n",off);
            }
#endif
/*各差分値(pack)を求める*/
/* 以下のループは並列化可能*/
#ifdef OMP
#pragma omp parallel for private(i,j,end_p)
#endif
            for(i=0;i<group_num;i++){
                int pos=0;
                int pos_bit=0;
                unsigned int diff_v,diff_v1;
                int start_p;

#ifdef DEBUG
                if (debug > 1 ){
                    printf("group_no= %d group_width=%d group_ref=%d\n",i,group_width[i],group_ref[i]);
                }
#endif
                start_p=i*group_size;
                end_p=org_size2;
                if (i < (group_num -1 )){
                    end_p=start_p + group_size;
                }
                diff_v = PEEK_N_UI4(src+group_adr[i]);
                if (group_width[i] > 0){
                    for(j=start_p;j<end_p;j++){
                        if ((pos_bit+group_width[i]) > 32){
                            diff_v1=(( diff_v <<  pos_bit) >> (32 - group_width[i])) ;
                            pos += 4;
                            diff_v = PEEK_N_UI4(src+group_adr[i]+pos);
                            pack[j]=(diff_v1 | ( diff_v >> (64 - (pos_bit + group_width[i]))) ) +group_ref[i];
                            pos_bit = pos_bit+group_width[i] - 32;
                        }else if ((pos_bit+group_width[i]) != 32) {
                            pack[j]=(( diff_v <<  pos_bit) >> (32 - group_width[i])) +group_ref[i];
                            pos_bit = pos_bit+group_width[i];
                        } else{
                            pack[j]=(( diff_v <<  pos_bit) >> (32 - group_width[i]))  +group_ref[i];
/*ここがマスクに変えられるが遅くなる*/
/*                            pack[j]=(diff_v & mask2[group_width[i]])  +group_ref[i];*/
                            pos += 4;
                            diff_v = PEEK_N_UI4(src+group_adr[i]+pos);
                            pos_bit = 0;
                        }
                    }
                }else{
                    for(j=start_p;j<end_p;j++){
                        pack[j]=group_ref[i];
                    }
                }
            }
/*指数部は11bitなので特別処理をする*/
/*
            if (k==0){
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<org_size;i++){
                    if (pack[i] >=2048){
                        pack[i] -=2048; }
                }
            }
*/
            pack_offset2 = pack_offset * 2;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
            for(i=0;i<org_size2;i++){
                pack[i] -= pack_offset;
                if ( pack[i] < 0 ){
                    pack[i] += pack_offset2;
                }
            }
            unpack[0]=pack[0];
#ifdef SD
            unpack[1]=pack[1];
            for(i=2;i<org_size2;i++){
                unpack[i]=( 2*unpack[i-1] ) - unpack[i-2] + pack[i];
            }
#else
            for(i=1;i<org_size2;i++){
                unpack[i]=unpack[i-1] + pack[i];
            }
#endif
/* データを格納する*/
            if (k==0){
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<org_size;i++){
                    dst[i*8] = dst[i*8] | ((unpack[i] & 0x07f0) >>4);
                    dst[i*8+1] = (unpack[i]<<4);
                }
            }else if(k==1 ){ 
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<org_size;i++){
                    dst[i*8+1] = dst[i*8+1] | (unpack[i]>>12);
                    dst[i*8+2] = (unpack[i] >> 4);
                    dst[i*8+3] = (unpack[i] << 4);
                }
            }else if(k==2 ){ 
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<org_size;i++){
                    dst[i*8+3] = dst[i*8+3] | (unpack[i]>>12);
                    dst[i*8+4] = (unpack[i] >> 4);
                    dst[i*8+5] = (unpack[i] << 4);
                }
            }else if(k==3 ){ 
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<org_size;i++){
                    dst[i*8+5] = dst[i*8+5] | (unpack[i]>>12);
                    dst[i*8+6] = (unpack[i] >> 4);
                    dst[i*8+7] = (unpack[i] << 4);
                }
            }else{
#ifdef OMP
#pragma omp parallel for private(i,buf2)
#endif
                for (i=0;i<org_size;i+=4){
                    POKE_N_UI2(buf2,unpack[i/4]);
                    dst[i*8+7] = dst[i*8+7] | ((buf2[0] & 0x10) >>1) | ((buf2[0] & 0x01) <<2) | ((buf2[1] & 0x10) >>3 ) | ((buf2[1] & 0x01));
                    dst[i*8+15] = dst[i*8+15] | ((buf2[0] & 0x20) >>2) | ((buf2[0] & 0x02) <<1) | ((buf2[1] & 0x20) >>4 ) | ((buf2[1] & 0x02)>>1);
                    dst[i*8+23] = dst[i*8+23] | ((buf2[0] & 0x40) >>3) | ((buf2[0] & 0x04) ) | ((buf2[1] & 0x40) >>5 ) | ((buf2[1] & 0x04)>>2);
                    dst[i*8+31] = dst[i*8+31] | ((buf2[0] & 0x80) >>4) | ((buf2[0] & 0x08) >>1) | ((buf2[1] & 0x80) >>6 ) | ((buf2[1] & 0x08)>>3);
                }
                end_p = ((int)org_size/4)*4;
                if (end_p < org_size){
                    POKE_N_UI2(buf2,unpack[end_p/4]);
                    dst[end_p*8+7] = dst[end_p*8+7] | ((buf2[0] & 0x10) >>1) | ((buf2[0] & 0x01) <<2) | ((buf2[1] & 0x10) >>3 ) | ((buf2[1] & 0x01));
                }
                if ((end_p+1) < org_size){
                    POKE_N_UI2(buf2,unpack[end_p/4]);
                    dst[end_p*8+15] = dst[end_p*8+15] | ((buf2[0] & 0x20) >>2) | ((buf2[0] & 0x02) <<1) | ((buf2[1] & 0x20) >>4 ) | ((buf2[1] & 0x02)>>1);
                }
                if ((end_p+2) < org_size){
                    POKE_N_UI2(buf2,unpack[end_p/4]);
                    dst[end_p*8+23] = dst[end_p*8+23] | ((buf2[0] & 0x40) >>3) | ((buf2[0] & 0x04) ) | ((buf2[1] & 0x40) >>5 ) | ((buf2[1] & 0x04)>>2);
                }
            }
        }else if(flag_chk==2){
/*ランレングス圧縮を解凍する*/
            if (run_alloc_flag==0){
                if ((runlength_data = (unsigned char*)malloc(org_size*16)) == NULL){
                    printf("malloc error(runlength_data) size=%d\n",org_size*16);
                    exit(1);
                }
                run_alloc_flag=1;
            }
            if (k==0){
                if (rcs[0]==0){
                    for(i=0;i<7;i++){
                        bit_adr[i]=i+1;
                    }
                    for(i=7;i<11;i++){
                        bit_adr[i]=i-7;
                    }
                    off2= off;
                    first_adr[0] = off + 44;
                    for(i=1;i<11;i++){
                        first_adr[i] =first_adr[i-1] + PEEK_N_UI4(src+off+(i-1)*4);
                    }
                    for(j=0;j<11;j++){
                        off += PEEK_N_UI4(src+off2+j*4);
                    }
                    off += 44;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                    for(i=0;i<11;i++){
#ifdef DEBUG
                        if (debug > 0){
                            printf("RunLength Decode(%d) Part=%d packing adr=%d %u\n",i,k,first_adr[i],src[first_adr[i]]);
                        }
#endif
                        rcs[i]=decode_runlength((unsigned char *)src+first_adr[i],runlength_data+org_size*i,org_size,1,0,bit_adr[i]);
                    }
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                    for(i=0;i<org_size;i++){
                        dst[i*8] = (dst[i*8] | runlength_data[i] | runlength_data[i+org_size] | runlength_data[i+org_size*2] | runlength_data[i+org_size*3] | runlength_data[i+org_size*4] | runlength_data[i+org_size*5] |runlength_data[i+org_size*6] ); 
                        dst[i*8+1] = (runlength_data[i+org_size*7] | runlength_data[i+org_size*8] | runlength_data[i+org_size*9] | runlength_data[i+org_size*10]); 
                    }
                }else{
/*既にランレングス解凍は符号部と一緒にやっている*/
                    off2= off;
                    for(j=0;j<11;j++){
                        off += PEEK_N_UI4(src+off2+j*4);
                    }
                    off += 44;
                }
            }else if(k !=4){
                for(i=0;i<4;i++){
                    bit_adr[i]=i+4;
                    bit_adr[i+12]=i;
                }
                for(i=4;i<12;i++){
                    bit_adr[i]=i-4;
                }
                off2= off;
                first_adr[0] = off + 64;
                for(i=1;i<16;i++){
                    first_adr[i] =first_adr[i-1] + PEEK_N_UI4(src+off+(i-1)*4);
                }
                for(j=0;j<16;j++){
                    off += PEEK_N_UI4(src+off2+j*4);
                }
                off += 64;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<16;i++){
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Decode(%d) Part=%d packing first_adr=%d\n",i,k,first_adr[i]);
                    }
#endif
                    rcs[i]=decode_runlength((unsigned char *)src+first_adr[i],runlength_data+org_size*i,org_size,1,0,bit_adr[i]);
                    }
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<org_size;i++){
                    dst[i*8+k*2-1] = (dst[i*8+k*2-1] | runlength_data[i] | runlength_data[i+org_size] | runlength_data[i+org_size*2] | runlength_data[i+org_size*3]); 
                    dst[i*8+k*2] = (runlength_data[i+org_size*4] | runlength_data[i+org_size*5] | runlength_data[i+org_size*6] | runlength_data[i+org_size*7] | runlength_data[i+org_size*8] | runlength_data[i+org_size*9] |runlength_data[i+org_size*10] | runlength_data[i+org_size*11]); 
                    dst[i*8+k*2+1] = (runlength_data[i+org_size*12] | runlength_data[i+org_size*13] | runlength_data[i+org_size*14] | runlength_data[i+org_size*15]); 
                }
            }else{
                for(i=0;i<4;i++){
                    bit_adr[i]=i+4;
                }
                off2= off;
                first_adr[0] = off + 16;
                for(i=1;i<4;i++){
                    first_adr[i] =first_adr[i-1] + PEEK_N_UI4(src+off+(i-1)*4);
                }
                for(j=0;j<4;j++){
                    off += PEEK_N_UI4(src+off2+j*4);
                }
                off += 16;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<4;i++){
                    rcs[i]=decode_runlength((unsigned char *)src+first_adr[i],runlength_data+org_size*i,org_size,1,0,bit_adr[i]);
                }
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for(i=0;i<org_size;i++){
                    dst[i*8+7] = (dst[i*8+7] | runlength_data[i] | runlength_data[i+org_size] | runlength_data[i+org_size*2] | runlength_data[i+org_size*3]); 
                }
            }
        }else{
/*圧縮が効かなかった悲しいケース*/
#ifdef DEBUG
            if (debug > 0){
                printf("UnPacking Part=%d No packing offset=%d\n",k,off);
            }
#endif
            if (k==0){
                off +=nopack_restore(src+off, dst, org_size, 0, 11,8);
            }else if(k!=4){
                off +=nopack_restore(src+off, dst, org_size, k*2, 16,8);
            }else{
                off +=nopack_restore(src+off, dst, org_size, 0, 4,8);
            }
        }
    }
    if (little_endian == 1){
        unsigned char swap;
#ifdef OMP
#pragma omp parallel for private(i,j,swap)
#endif
        for(i=0;i<org_size*8;i+=8){
            for(j=0;j<4;j++){
                swap = dst[i+j];
                dst[i+j] = dst[i+7-j];
                dst[i+7-j] = swap;
            }
        }
    }
#ifdef DEBUG
    if (debug > 0){
        if (pack_size != off){
            printf("X Unpack Size Err store=%d off =%d\n",pack_size,off);
            exit(1);
        }
    }
#endif
    if (run_alloc_flag==1){
        free(runlength_data);
    }
    if (com_alloc_flag==1){
        free(group_width);
        free(group_ref);
        free(group_adr);
        free(pack);
        free(unpack);
    }
    return (org_size);
}
int encode_r4(unsigned char *src,/*元データ*/
              unsigned char *out,/*圧縮したものを詰め込む配列*/
              int size,/*元データの要素数*/
              int little_endian/*0: big endian 1:little_endian*/
              ){
    int i,j,k,group_num;
    unsigned short *unpack,*pack2,*group_ref,*group_width;
    unsigned int *raw;
    int *raw_pos,off,off_old;
    unsigned char flag1=0;/* 9バイト目に格納される*/
    unsigned char flag2=0;/* 10バイト目に格納される*/
    unsigned  char *buf;
    int end_p;
    int skip =0;/*0:複合差分圧縮 -1:ランレングス圧縮 1:非圧縮*/
    int pack_size=0,org_size;
    int pack_offset ;
/*                          0                  10                  20                  30  */
    const int byte_adr[32]={0,1,1,3,0,0,0,0,0,0,0,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3};
    const int bit_adr[32] ={0,0,7,7,1,2,3,4,5,6,7,1,2,3,4,5,6,0,1,2,3,4,5,6,7,0,1,2,3,4,5,6};
    int rcs_t[32];
    unsigned char *runlength_data;
    int threads=1;
    int threads2,start_threads,end_threads;
    int run_offset;
    int add=0;

#ifdef OMP
    threads=omp_get_max_threads();
#endif

    group_num=((size-1)/group_size+1);
    if ((runlength_data=(unsigned char*)malloc(size*8+32)) == NULL){
        printf("malloc error(runlength_data) size=%d\n",size*2);
        exit(1);
    }
    if ((unpack=(unsigned short*)malloc(size*2)) == NULL){
        printf("malloc error(unpack) size=%d\n",size*2);
        exit(1);
    }
    if ((pack2=(unsigned short*)malloc(size*2)) == NULL){
        printf("malloc error(pack2) size=%d\n",size*2);
        exit(1);
    }
    if ((raw=(unsigned int*)malloc(size*4)) == NULL){
        printf("malloc error(raw) size=%d\n",size*4);
        exit(1);
    }
    if ((raw_pos=(int*)malloc(size*4)) == NULL){
        printf("malloc error(raw_pos) size=%d\n",size*4);
        exit(1);
    }
    if ((group_width=(unsigned short*)malloc(group_num*2)) == NULL){
        printf("malloc error(group_width) size=%d\n",group_num*2);
        exit(1);
    }
    if ((group_ref=(unsigned short*)malloc(group_num*2)) == NULL){
        printf("malloc error(group_ref) size=%d\n",group_num*2);
        exit(1);
    }
    if (little_endian == 1){
        if ((buf=(unsigned char *)malloc(size*8)) == NULL){
            printf("malloc error(buf) size=%d\n",group_num*2);
            exit(1);
        }
#ifdef OMP
#pragma omp parallel for private(i,j)
#endif
        for(i=0;i<size*4;i+=4){
            for(j=0;j<4;j++){
                buf[i+(3-j)] = src[i+j];
            }
        }
    }else{
        buf = src;
    }
/* Output size*/
    off = 0;
/*最初の4バイトに(big endian)資料長を入れる。*/
    POKE_N_SI4(out+off,size);
    off +=4;
/* 後でoffを入れるので開けておく*/
    off +=4;
/*フラグを入れるので開けておく*/
    off +=2;
/*指数部,仮数部の先頭アドレスを入れるので開けておく*/
    off +=12;
    off_old=off;
/*　スレッド数によりランレングス圧縮の範囲を変える*/
/*符号,指数部最下位,2箇所の仮数部最下位bit,指数部残り6bit、残りを上位bitからランレングス圧縮してみる*/
    if (threads > 32){
        threads2=32;
    }else if (threads ==3){
#if defined(F_RUN)
        threads2=4;
#else
        threads2=3;
#endif
    }else if (threads < 3){
#if defined(F_RUN)
        threads2=4;
#else
        threads2=2;
#endif
    }else{
        threads2=threads;
    }
#ifdef OMP
#pragma omp parallel for private(i,run_offset)
#endif
    for (i=0;i<threads2;i++){
        run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
        rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,4,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
        if (debug > 0){
            printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d\n",i,rcs_t[i],(size-1)/4 + 1,byte_adr[i],bit_adr[i]);
        }
#endif
    }
    if (threads2 < 4){
        if (threads == 1){
            for (i=2;i<4;i++){
/*圧縮効率が認められた時のみランレングス圧縮を続ける*/
                if (((float)(rcs_t[i-1]+2)/((size-1)/8 + 1)*100.0) < RUNLENGTH_LIMIT){
                    run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
                    rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,4,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d\n",i,rcs_t[i],(size-1)/4 + 1,byte_adr[i],bit_adr[i]);
                    }
#endif
                }else{
                    rcs_t[i] = rcs_t[i-1];
                }
            }
            threads2=4;
        }else{
/*圧縮効率が認められた時のみランレングス圧縮を続ける*/
            if (((float)(rcs_t[threads2-1]+2)/((size-1)/8 + 1)*100.0) < RUNLENGTH_LIMIT){
#ifdef OMP
#pragma omp parallel for private(i,run_offset)
#endif
                for (i=threads2;i<threads2+threads;i++){
                    run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
                    rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,4,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d\n",i,rcs_t[i],(size-1)/8 + 1,byte_adr[i],bit_adr[i]);
                    }
#endif
                }
                threads2 += threads;
            }else{
                for (i=threads2;i<4;i++){
                    rcs_t[i] = rcs_t[threads2-1];
                }
                threads2 = 4;
            }
        }
    }
    start_threads=threads2;
#if defined(S_RUN)
/*(デバック用) 必ず圧縮*/
if (1 < 0){
#elif defined(S_NO)
/*(デバック用)必ず非圧縮*/
if (1 > 0){
#else
    if (rcs_t[0] > ((size-1)/4 + 1)){
#endif
/*不幸にも圧縮するとサイズが大きくなった場合そのまま詰める*/
#ifdef DEBUG
        if (debug > 0){
            printf("Not Exec Run Length Encooding pack_size=%d org_size =%d \n",off-off_old,(size-1)/4 + 1);
        }
#endif
        off += nopack_store(buf, out+off, size, 0, 1,4);
    }else{
/*符号部は圧縮が効いたのでランレングス圧縮*/
        flag1 = 0x80;
        memcpy(out+off,runlength_data,rcs_t[0]);
        off += rcs_t[0];
    }
#ifdef DEBUG
    if (debug > 0){
        printf("Part 00 Packing_size=%d Org_size=%d Rate =%5.2f(%%) offset=%d \n",off-off_old,(size-1)/4+1,(float)(off-off_old)/((size-1)/4+1)*100.0,off);
    }
#endif
/* 圧縮処理
 k = 0 指数部(8bit)
 k = 1 仮数部1(最上位7bit)
 k = 2 仮数部2(16bit)
 */
    for (k=0;k<3;k++){
/*指数部,仮数部の先頭アドレスをいれておく*/
        POKE_N_SI4(out+10+k*4,off);
        off_old =off;
        if (((float)(rcs_t[k+1]+2)/((size-1)/8 + 1)*100.0) < RUNLENGTH_LIMIT){ 
            skip = -1;
        }else if (skip !=1 || k != 2){
            skip = 0;
        }
#if defined(E_RUN)
        if (k==0){
            skip =-1;
        }
#elif defined(E_COM)
        if (k==0){
            skip =0;
        }
#elif defined(E_NO)
        if (k==0){
            skip =1;
        }
#endif
#if defined(F_RUN)
        if (k!=0){
            skip =-1;
        }
#elif defined(F_COM)
        if (k!=0){
            skip =0;
        }
#elif defined(F_NO)
        if (k!=0){
            skip =1;
        }
#endif
/*
ここからが複合差分圧縮本体
*/
/*ランレングス圧縮が効かない時には複合差分圧縮をしてみよう*/
        if (skip==0){
#ifdef DEBUG
            if (debug > 0){
                printf("part %d Packing start offset=%d\n",k,off);
            }
#endif
/*まずは、元データをunpackにセットする*/
            if (k== 0){
                pack_offset=pack_offset8;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for (i=0;i<size;i++){
                    unpack[i] = ((buf[i*4] & 0x7f) <<1 ) | (buf[i*4+1] >> 7);
                }
            }else if(k==1 ){ 
                pack_offset=pack_offset7;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for (i=0;i<size;i++){
                    unpack[i] = buf[i*4+1] & 0x7f; 
                }
            }else if(k==2 ){ 
                pack_offset=pack_offset16;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for (i=0;i<size;i++){
                    unpack[i] = (buf[i*4+2]  <<8 ) | buf[i*4+3] ;
                }
            }
            pack2[0] = unpack[0] + pack_offset;
#ifdef SD
            pack2[1] = unpack[1] + pack_offset;
#endif

/* 16bitの範囲に収まるようにpackを循環させる*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
#ifdef SD
            for (i=2;i<(size);i++){
                pack2[i] = unpack[i] - ( 2 * unpack[i-1]) + unpack[i-2] +pack_offset;
            }
#else
            for (i=1;i<(size);i++){
                pack2[i] = unpack[i] -  unpack[i-1] +pack_offset;
            }
#endif
            encode_complex(pack2, raw, size, group_num, group_width, group_ref, raw_pos);
/* Output group_ref*/
#ifdef DEBUG
            if (debug > 1){
                printf("group_ref offset=%d \n",off);
            }
#endif
/*格納する前に、圧縮前よりサイズが大きくならないかチェックする*/
            if (k==0){
                pack_size  =group_num;
                pack_size +=(group_num-1)*4/8 +1;
#ifdef OMP
#pragma omp parallel for reduction(+:pack_size)
#endif
                for (i=0;i<group_num;i++){
                    pack_size += ((raw_pos[i] - i*group_size)*4);
                }
                org_size = size;
            }else if (k==1){
                pack_size =(group_num-1)*7/8 +1;
                pack_size +=(group_num-1)*4/8 +1;
#ifdef OMP
#pragma omp parallel for reduction(+:pack_size)
#endif
                for (i=0;i<group_num;i++){
                    pack_size += ((raw_pos[i] - i*group_size)*4);
                }
                org_size = (size-1)*7/8 +1;
            }else{
                pack_size  =group_num*2;
                pack_size +=(group_num-1)*4/8 +1;
#ifdef OMP
#pragma omp parallel for reduction(+:pack_size)
#endif
                for (i=0;i<group_num;i++){
                    pack_size += ((raw_pos[i] - i*group_size)*4);
                }
                org_size = size*2;
            }
#if defined(E_COM) & defined(F_COM)
/*(デバック用) 必ず複合差分圧縮*/
if ( 1 > 0){
#elif defined(E_COM)
if ( k==0 || (skip==0 && pack_size < org_size)){ 
#elif defined(F_COM) 
if ( k!=0 || (skip==0 && pack_size < org_size)){ 
#else
            if (skip==0 && pack_size < org_size){
#endif
/*(デバック用)必ず非圧縮/ランレングス*/
#if defined(E_NO) | defined(E_RUN)
                if (k==0){goto OUT1;} 
#endif
#if defined(F_NO) | defined(F_RUN) 
                if (k!=0){goto OUT1;} 
#endif
                if (k==0){
/* 指数部は8bitで格納する*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                    for (i=0;i<group_num;i++){
                        out[off+i]  =group_ref[i];
                    }
                    off +=group_num;
                }else if (k==1){
/* 仮数部1は7bitで格納する*/
/*                    for(i=0;i<(group_num-1)*11/8+1;i++){
                        out[off+i]=0;
                    }*/
                    end_p=group_num/8;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                    for(i=0;i<end_p;i++){
                        out[off+i*7]  =(group_ref[i*8] << 1) | ((group_ref[i*8+1] >>6) & 0x01);
                        out[off+i*7+1]=(group_ref[i*8+1] << 2) | ((group_ref[i*8+2] >>5) & 0x03);
                        out[off+i*7+2]=(group_ref[i*8+2] << 3) | ((group_ref[i*8+3] >>4) & 0x07);
                        out[off+i*7+3]=(group_ref[i*8+3] << 4) | ((group_ref[i*8+4] >>3) & 0x0f);
                        out[off+i*7+4]=(group_ref[i*8+4] << 5) | ((group_ref[i*8+5] >>2) & 0x1f);
                        out[off+i*7+5]=(group_ref[i*8+5] << 6) | ((group_ref[i*8+6] >>1) & 0x3f);
                        out[off+i*7+6]=(group_ref[i*8+6] << 7) | (group_ref[i*8+7] & 0x7f);
                    }
                    off += end_p*7;
                    add=0;
/*余り*/
                    if (end_p*8 < group_num){
                        out[off]  =(group_ref[end_p*8] << 1) ;
                        add=1; 
                    }
                    if ((end_p*8+1) < group_num){
                        out[off]  =out[off] | ((group_ref[end_p*8+1] >>6) & 0x01);
                        out[off+1]=(group_ref[end_p*8+1] << 2);
                        add= 2;
                    }
                    if ((end_p*8+2) < group_num){
                        out[off+1]=out[off+1] | ((group_ref[end_p*8+2] >>5) & 0x03);
                        out[off+2]=(group_ref[end_p*8+2] << 3);
                        add= 3;
                    }
                    if ((end_p*8+3) < group_num){
                        out[off+2]=out[off+2] | ((group_ref[end_p*8+3] >>4) & 0x07);
                        out[off+3]=(group_ref[end_p*8+3] << 4) ;
                        add= 4;
                    }
                    if ((end_p*8+4) < group_num){
                        out[off+3]=out[off+3] | ((group_ref[end_p*8+4] >>3) & 0x0f);
                        out[off+4]=(group_ref[end_p*8+4] << 5);
                        add= 5;
                    }
                    if ((end_p*8+5) < group_num){
                        out[off+4]=out[off+4] | ((group_ref[end_p*8+5] >>2) & 0x1f);
                        out[off+5]=(group_ref[end_p*8+5] << 6) ;
                        add= 6;
                    }
                    if ((end_p*8+6) < group_num){
                        out[off+5]=out[off+5] | ((group_ref[end_p*8+6] >>1) & 0x3f);
                        out[off+6]=(group_ref[end_p*8+6] << 7);
                        add= 7;
                    }
                    off +=add;
                }else{
/* 仮数部2は16bitで格納する*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                    for (i=0;i<group_num;i++){
                        POKE_N_UI2(out+off+2*i,group_ref[i]);
                    }
                    off +=(2*group_num);
                }
#ifdef DEBUG
                if (debug > 1){
                    printf("group_width offset=%d\n",off);
                }
#endif
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for (i=0;i<(group_num-1);i=i+2){
                    out[off+i/2]=group_width[i]<< 4 | group_width[i+1];
                }
/*余りビット処理*/
                if ((group_num % 2 ) == 1){
                    out[off+(group_num/2)] = group_width[group_num-1] <<4;
                }
                off += ((group_num-1) / 2 + 1);
#ifdef DEBUG
                if (debug > 1){
                    printf("raw offset=%d %d \n",off,group_num);
                }
#endif
                for (i=0;i<group_num;i++){
/*圧縮されたデータは32bit単位になっているはず*/
                    for (j=i*group_size;j<raw_pos[i];j++){
                        POKE_N_UI4(out+off,raw[j]);
                        off += 4;
                    }
                }
            }else{
                skip =1;
            }
        }
OUT1:
/*ここからが、指数部と仮数部の複合差分圧縮後の処理*/
/*指数部*/
        if (k==0){
            if(skip==0){
/*複合差分圧縮*/
                flag1 = flag1 | 0x10;
            }else if(skip==-1){
/*ランレングス圧縮*/
                end_threads=start_threads;
                while (end_threads < 11){
                    end_threads += threads;
                }
                if (end_threads > 32 ){ end_threads=32;}
#ifdef OMP
#pragma omp parallel for private(i,run_offset)
#endif
                for (i=start_threads;i<end_threads;i++){
                    run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
                    rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,4,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d \n",i,rcs_t[i],(size-1)/8 + 1,byte_adr[i],bit_adr[i]);
                    }
#endif
                }
                for (i=0;i<7;i++){
                    POKE_N_UI4(out+off+i*4,rcs_t[i+4]);
                }
                off += 28;
                POKE_N_UI4(out+off,rcs_t[1]);
                off += 4;
                for (i=0;i<7;i++){
                    memcpy(out+off,runlength_data+(size/4+1)*(i+1),rcs_t[i+4]);
                    off += rcs_t[i+4];
                }
                memcpy(out+off,runlength_data+(size/4+1)*8,rcs_t[1]);
                off += rcs_t[1];
                flag1 = flag1 | 0x20;
                start_threads=end_threads;
            } else {
/*非圧縮*/
#ifdef DEBUG
                if (debug > 0){
                    printf("Not Exec Complex Packing and Spatial Differnecing Schemes k=%d pack_size=%d org_size =%d \n",k,pack_size,org_size);
                }
#endif
                off += nopack_store(buf, out+off, size, 0, 8,4);
                skip =0; /*指数部と仮数部は圧縮傾向が異なるため一旦リセット*/
            }
        }else if (k==1){
/*仮数部1*/
            if(skip==-1){
/*ランレングス圧縮*/
                if (start_threads < 11){
                    start_threads =11;
                }
                end_threads=start_threads;
                while (end_threads < 17){
                    end_threads += threads;
                }
                if (end_threads > 32){ end_threads=32;}
#ifdef OMP
#pragma omp parallel for private(i,run_offset)
#endif
                for (i=start_threads;i<end_threads;i++){
                    run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
                    rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,4,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d \n",i,rcs_t[i],(size-1)/8 + 1,byte_adr[i],bit_adr[i]);
                    }
#endif
                }
                for (i=0;i<6;i++){
                    POKE_N_UI4(out+off+i*4,rcs_t[i+11]);
                }
                off += 24;
                POKE_N_UI4(out+off,rcs_t[2]);
                off += 4;
                for (i=0;i<6;i++){
                    memcpy(out+off,runlength_data+(size/4+1)*(9+i),rcs_t[i+11]);
                    off += rcs_t[i+11];
                }
                memcpy(out+off,runlength_data+(size/4+1)*15,rcs_t[2]);
                off += rcs_t[2];
                start_threads=end_threads;
                flag1 = flag1 | 0x08;
            }else if (skip==1){
/*非圧縮*/
#ifdef DEBUG
                if (debug > 0){
                    printf("Not Exec Complex Packing and Spatial Differnecing Schemes k= %d pack_size=%d org_size =%d \n",k,pack_size,org_size);
                }
#endif
                off = off_old;
                off += nopack_store(buf, out+off, size, 0, 7,4);
            }else{ 
/*複合差分圧縮*/
                flag1 = flag1 | 0x04;
            }
        }else{
/*最下位16bitの仮数部*/
            if(skip==-1){
/*ランレングス圧縮*/
                if (start_threads < 17){
                    start_threads =17;
                }
                end_threads=32;
#ifdef OMP
#pragma omp parallel for private(i,run_offset)
#endif
                for (i=start_threads;i<end_threads;i++){
                    run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
                    rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,4,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d\n",i,rcs_t[i],(size-1)/8 + 1,byte_adr[i],bit_adr[i]);
                    }
#endif
                }
                for (i=0;i<15;i++){
                    POKE_N_UI4(out+off+i*4,rcs_t[i+17]);
                }
                off += 60;
                POKE_N_UI4(out+off,rcs_t[3]);
                off += 4;
                for (i=0;i<15;i++){
                    memcpy(out+off,runlength_data+(size/4+1)*(i+16),rcs_t[i+17]);
                    off += rcs_t[i+17];
                }
                memcpy(out+off,runlength_data+(size/4+1)*31,rcs_t[3]);
                off += rcs_t[3];
                flag1 = flag1 | 0x02;
            }else if (skip==1){
/*非圧縮*/
#ifdef DEBUG
                if (debug > 0){
                    printf("Not Exec Complex Packing and Spatial Differnecing Schemes k= %d pack_size=%d org_size =%d \n",k,pack_size,org_size);
                }
#endif
                off = off_old;
                off += nopack_store(buf, out+off, size, 2, 16,4);
            }else{
/*複合差分圧縮*/
                flag1 = flag1 | 0x01;
            }
        }
#ifdef DEBUG
        if (debug > 0){
            int bits ;
            if (k==0){ bits=8;
            }else if(k==1){ bits=7;
            }else if(k==2){ bits=16;
            }
            if (debug > 0){
                printf("Part %d Packing_size=%d Org_size=%d Rate =%5.2f(%%) offset=%d skip=%d\n",k, off-off_old,(size*bits-1)/8,(float)(off-off_old)/(size*bits-1)*8*100.0,off,skip);
            }
        }
#endif
    }
#ifdef DEBUG
    if (debug > 0){
        printf("Total Packing_size=%d Org_size=%d Rate =%f(%%) \n",off,size*8,(float)off/size/8*100.0);fflush(stdout);
    }
#endif

/*最後にデータ長とフラグを書き込む*/
    POKE_N_SI4(out+4,off);
    memcpy(out+8,&flag1,1);
    memcpy(out+9,&flag2,1);
    free(group_width);
    free(group_ref);
    free(pack2);
    free(unpack);
    free(raw);
    free(raw_pos);
    free(runlength_data);
    if (little_endian ==1){
        free(buf);
    }
    return (off);
}
int encode_r8(unsigned char *src,/*元データ*/
              unsigned char *out,/*圧縮したものを詰め込む配列*/
              int size,/*元データの要素数*/
              int little_endian/*0: big endian 1:little_endian*/
            ){
    int i,j,k,group_num;
    unsigned short *unpack,*pack2,*group_ref,*group_width;
    unsigned char buf2[4];
    unsigned int *raw;
    int *raw_pos,off,off_old;
    unsigned char flag1=0;/* 9バイト目に格納される*/
    unsigned char flag2=0;/* 10バイト目に格納される*/
    unsigned  char *buf;
    int end_p;
    int skip =0;/*0:複合差分圧縮 -1:ランレングス圧縮 1:非圧縮*/
    int pack_size=0,org_size;
    int pack_offset ;
/*                          0                  10                  20                  30                  40                  50                  60 */
    const int byte_adr[64]={0,1,3,5,7,7,0,0,0,0,0,0,0,1,1,1,1,1,1,1,2,2,2,2,2,2,2,2,3,3,3,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,6,6,6,6,6,6,6,6,7,7,7,7,7,7};
    const int bit_adr[64] ={0,3,3,3,3,7,1,2,3,4,5,6,7,0,1,2,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,4,5,6,7,0,1,2,3,4,5,6,7,0,1,2,4,5,6};
    int rcs_t[64];
    unsigned char *runlength_data;
    int threads=1;
    int threads2,start_threads,end_threads;
    int run_offset;
    int add=0;

#ifdef OMP
    threads=omp_get_max_threads();
#endif

    group_num=((size-1)/group_size+1);
    if ((runlength_data=(unsigned char*)malloc(size*16+64)) == NULL){
        printf("malloc error(runlength_data) size=%d\n",size*2);
        exit(1);
    }
    if ((unpack=(unsigned short*)malloc(size*2)) == NULL){
        printf("malloc error(unpack) size=%d\n",size*2);
        exit(1);
    }
    if ((pack2=(unsigned short*)malloc(size*2)) == NULL){
        printf("malloc error(pack2) size=%d\n",size*2);
        exit(1);
    }
    if ((raw=(unsigned int*)malloc(size*4)) == NULL){
        printf("malloc error(raw) size=%d\n",size*4);
        exit(1);
    }
    if ((raw_pos=(int*)malloc(size*4)) == NULL){
        printf("malloc error(raw_pos) size=%d\n",size*4);
        exit(1);
    }
    if ((group_width=(unsigned short*)malloc(group_num*2)) == NULL){
        printf("malloc error(group_width) size=%d\n",group_num*2);
        exit(1);
    }
    if ((group_ref=(unsigned short*)malloc(group_num*2)) == NULL){
        printf("malloc error(group_ref) size=%d\n",group_num*2);
        exit(1);
    }
    if (little_endian == 1){
        if ((buf=(unsigned char *)malloc(size*8)) == NULL){
            printf("malloc error(buf) size=%d\n",group_num*2);
            exit(1);
        }
#ifdef OMP
#pragma omp parallel for private(i,j)
#endif
        for(i=0;i<size*8;i+=8){
            for(j=0;j<8;j++){
                buf[i+(7-j)] = src[i+j];
            }
        }
    }else{
        buf = src;
    }
/* Output size*/
    off = 0;
/*最初の4バイトに(big endian)資料長を入れる。*/
    POKE_N_SI4(out+off,size);
    off +=4;
/* 後でoffを入れるので開けておく*/
    off +=4;
/*フラグを入れるので開けておく*/
    off +=2;
/*指数部,仮数部の先頭アドレスを入れるので開けておく*/
    off +=20;
    off_old=off;
/*　スレッド数によりランレングス圧縮の範囲を変える*/
/*符号,指数部最下位,4箇所の仮数部最下位bit,指数部残り10bit、残りを上位bitからランレングス圧縮してみる*/
    if (threads > 64){
        threads2=64;
    }else if (threads < 6 && threads > 2){
#if defined(F_RUN)
        threads2=6;
#else
        threads2=threads;
#endif
    }else if (threads < 3){
#if defined(F_RUN)
        threads2=6;
#else
        threads2=2;
#endif
    }else{
        threads2=threads;
    }
#ifdef OMP
#pragma omp parallel for private(i,run_offset)
#endif
    for (i=0;i<threads2;i++){
        run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
        rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,8,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
        if (debug > 0){
            printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d\n",i,rcs_t[i],(size-1)/8 + 1,byte_adr[i],bit_adr[i]);fflush(stdout);
        }
#endif
    }
    if (threads2 < 6){
        if (threads == 1){
            for (i=2;i<6;i++){
/*圧縮効率が認められた時のみランレングス圧縮を続ける*/
                if (((float)(rcs_t[i-1]+2)/((size-1)/8 + 1)*100.0) < RUNLENGTH_LIMIT){
                    run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
                    rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,8,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d\n",i,rcs_t[i],(size-1)/8 + 1,byte_adr[i],bit_adr[i]);fflush(stdout);
                    }
#endif
                }else{
                    rcs_t[i] = rcs_t[i-1];
                }
            }
            threads2 = 6;
        }else if (threads == 2){
/*圧縮効率が認められた時のみランレングス圧縮を続ける*/
            if (((float)(rcs_t[1]+2)/((size-1)/8 + 1)*100.0) < RUNLENGTH_LIMIT){
#ifdef OMP
#pragma omp parallel for private(i,run_offset)
#endif
                for (i=2;i<4;i++){
                    run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
                    rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,8,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d\n",i,rcs_t[i],(size-1)/8 + 1,byte_adr[i],bit_adr[i]);
                    }
#endif
                }
                if (((float)(rcs_t[3]+2)/((size-1)/8 + 1)*100.0) < RUNLENGTH_LIMIT){
#ifdef OMP
#pragma omp parallel for private(i,run_offset)
#endif
                    for (i=4;i<6;i++){
                        run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
                        rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,8,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
                        if (debug > 0){
                            printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d\n",i,rcs_t[i],(size-1)/8 + 1,byte_adr[i],bit_adr[i]);
                        }
#endif
                    }
                }else{
                    rcs_t[4] = rcs_t[3];
                    rcs_t[5] = rcs_t[3];
                }
            }else{
                for (i=2;i<6;i++){
                    rcs_t[i] = rcs_t[1];
                }
            }
            threads2 = 6;
        }else{
/*圧縮効率が認められた時のみランレングス圧縮を続ける*/
            if (((float)(rcs_t[threads2-1]+2)/((size-1)/8 + 1)*100.0) < RUNLENGTH_LIMIT){
#ifdef OMP
#pragma omp parallel for private(i,run_offset)
#endif
                for (i=threads2;i<threads2+threads;i++){
                    run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
                    rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,8,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d\n",i,rcs_t[i],(size-1)/8 + 1,byte_adr[i],bit_adr[i]);
                    }
#endif
                }
                threads2 += threads;
            }else{
                for (i=threads2;i<6;i++){
                    rcs_t[i] = rcs_t[threads2-1];
                }
                threads2 = 6;
            }
        }
    }
    start_threads=threads2;
#if defined(S_RUN)
/*(デバック用) 必ず圧縮*/
if (1 < 0){
#elif defined(S_NO)
/*(デバック用)必ず非圧縮*/
if (1 > 0){
#else
    if (rcs_t[0] > ((size-1)/8 + 1)){
#endif
/*不幸にも圧縮するとサイズが大きくなった場合そのまま詰める*/
#ifdef DEBUG
        if (debug > 0){
            printf("Not Exec Run Length Encooding pack_size=%d org_size =%d \n",off-off_old,(size-1)/8 + 1);
        }
#endif
        off += nopack_store(buf, out+off, size, 0, 1,8);
    }else{
/*符号部は圧縮が効いたのでランレングス圧縮*/
        flag1 = 0x80;
        memcpy(out+off,runlength_data,rcs_t[0]);
        off += rcs_t[0];
    }
#ifdef DEBUG
    if (debug > 0){
        printf("Part 00 Packing_size=%d Org_size=%d Rate =%5.2f(%%) offset=%d \n",off-off_old,(size-1)/8+1,(float)(off-off_old)/((size-1)/8+1)*100.0,off);
    }
#endif
/* 圧縮処理
 k = 0 指数部(11bit)
 k = 1 仮数部1(最上位16bit)
 k = 2 仮数部2
 k = 3 仮数部3
 k = 4 仮数部4(4bit)
 */
    for (k=0;k<5;k++){
/*指数部,仮数部の先頭アドレスをいれておく*/
        POKE_N_SI4(out+10+k*4,off);
        off_old =off;
        if (k==0){
            if (((float)(rcs_t[1]+2)/((size-1)/8 + 1)*100.0) < RUNLENGTH_LIMIT){
                skip = -1;
            }
#if defined(E_RUN)
            skip =-1;
#elif defined(E_COM)
            skip =0;
#elif defined(E_NO)
            skip =1;
#endif
        }else{
            if (((float)(rcs_t[k+1]+2)/((size-1)/8 + 1)*100.0) < RUNLENGTH_LIMIT){
                skip = -1;
            }else if (skip !=1){
                skip = 0;
            }
#if defined(F_RUN)
            skip =-1;
#elif defined(F_COM)
            skip =0;
#elif defined(F_NO)
            skip =1;
#endif
        }
/*
ここからが複合差分圧縮本体
*/
/*ランレングス圧縮が効かない時には複合差分圧縮をしてみよう*/
        if (skip==0){
#ifdef DEBUG
            if (debug > 0){
                printf("part %d Packing start offset=%d\n",k,off);
            }
#endif
/*まずは、元データをunpackにセットする*/
            if (k== 0){
                pack_offset=pack_offset11;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for (i=0;i<size;i++){
                    unpack[i] = ((buf[i*8] & 0x7f) <<4 ) | (buf[i*8+1] >> 4);
                }
            }else if(k==1 ){ 
                pack_offset=pack_offset16;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for (i=0;i<size;i++){
                    unpack[i] = (buf[i*8+1]  <<12 ) | (buf[i*8+2] << 4) |  (buf[i*8+3] >> 4);
                }
            }else if(k==2 ){ 
                pack_offset=pack_offset16;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for (i=0;i<size;i++){
                    unpack[i] = (buf[i*8+3]  <<12 ) | (buf[i*8+4] << 4) |  (buf[i*8+5] >> 4);
                }
            }else if(k==3 ){ 
                pack_offset=pack_offset16;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for (i=0;i<size;i++){
                    unpack[i] = (buf[i*8+5]  <<12 ) | (buf[i*8+6] << 4) |  (buf[i*8+7] >> 4);
                }
            }else{
/*仮数部最下位4bitは4データ分を一緒に圧縮する*/
/*　順序は、圧縮を効きやすくするため
データ1(60bit),データ2(60bit),...データ3(63bit),データ4(63bit)
の順に並べる*/
                pack_offset=pack_offset16;
                end_p = (int)(size / 4 ) * 4;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for (i=0;i<end_p;i+=4){
                    unpack[i/4]=((buf[i*8+7] & 0x08)<< 9) |((buf[i*8+7] & 0x04) << 6)| ((buf[i*8+7] & 0x02)<<3)| ((buf[i*8+7] & 0x01)) \
                     | ((buf[(i+1)*8+7] & 0x08)<< 10) |((buf[(i+1)*8+7] & 0x04) << 7)| ((buf[(i+1)*8+7] & 0x02)<<4)| ((buf[(i+1)*8+7] & 0x01)<< 1) \
                     | ((buf[(i+2)*8+7] & 0x08)<< 11) |((buf[(i+2)*8+7] & 0x04) << 8)| ((buf[(i+2)*8+7] & 0x02)<<5)| ((buf[(i+2)*8+7] & 0x01)<< 2) \
                     | ((buf[(i+3)*8+7] & 0x08)<< 12) |((buf[(i+3)*8+7] & 0x04) << 9)| ((buf[(i+3)*8+7] & 0x02)<<6)| ((buf[(i+3)*8+7] & 0x01)<< 3);
                }
/*余りの処理*/
                if (end_p <size){
                    buf2[0]=((buf[end_p*8+7] & 0x04)>>2)| ((buf[end_p*8+7] & 0x08)<< 1);
                    buf2[1]=((buf[end_p*8+7] & 0x01))| ((buf[end_p*8+7] & 0x02)<< 3);
                    if ((end_p+1) < size){
                        buf2[0]=((buf[(end_p+1)*8+7] & 0x04)>>1)| ((buf[(end_p+1)*8+7] & 0x08)<< 2) | buf2[0];
                        buf2[1]=((buf[(end_p+1)*8+7] & 0x01)<<1)| ((buf[(end_p+1)*8+7] & 0x02)<< 4) | buf2[1];
                    }
                    if ((end_p+2) < size){
                        buf2[0]=((buf[(end_p+2)*8+7] & 0x04))| ((buf[(end_p+2)*8+7] & 0x08)<< 3) | buf2[0];
                        buf2[1]=((buf[(end_p+2)*8+7] & 0x01)<<2)| ((buf[(end_p+2)*8+7] & 0x02)<< 5) | buf2[1];
                    }
                    unpack[end_p/4]=PEEK_N_UI2(buf2);
                }
                group_num=((size-1)/(group_size*4)+1);
            }
            pack2[0] = unpack[0] + pack_offset;
#ifdef SD
            pack2[1] = unpack[1] + pack_offset;
#endif

/* 16bitの範囲に収まるようにpackを循環させる*/
#ifdef OMP
#pragma omp parallel for private(i)
#endif
#ifdef SD
            for (i=2;i<(size);i++){
                pack2[i] = unpack[i] - ( 2 * unpack[i-1]) + unpack[i-2] +pack_offset;
            }
#else
            for (i=1;i<(size);i++){
                pack2[i] = unpack[i] - unpack[i-1] +pack_offset;
            }
#endif
            if (k!=4){
                encode_complex(pack2, raw, size, group_num, group_width, group_ref, raw_pos);
            }else{
                encode_complex(pack2, raw, ((size-1)/4 + 1), group_num, group_width, group_ref, raw_pos);
            }
/* Output group_ref*/
#ifdef DEBUG
            if (debug > 1){
                printf("group_ref offset=%d\n",off);
            }
#endif
/*格納する前に、圧縮前よりサイズが大きくならないかチェックする*/
            if (k==0){
                pack_size  =(group_num-1)*11/8 +1;
                pack_size +=(group_num-1)*4/8 +1;
#ifdef OMP
#pragma omp parallel for reduction(+:pack_size)
#endif
                for (i=0;i<group_num;i++){
                    pack_size += ((raw_pos[i] - i*group_size)*4);
                }
                org_size = ((size*11-1)/8)+1;
            }else if (k==4){
                pack_size  =group_num*2;
                pack_size +=(group_num-1)*4/8 +1;
#ifdef OMP
#pragma omp parallel for reduction(+:pack_size)
#endif
                for (i=0;i<group_num;i++){
                    pack_size += ((raw_pos[i] - i*group_size)*4);
                }
                org_size = (size-1)/2 +1;
            }else{
                pack_size  =group_num*2;
                pack_size +=(group_num-1)*4/8 +1;
#ifdef OMP
#pragma omp parallel for reduction(+:pack_size)
#endif
                for (i=0;i<group_num;i++){
                    pack_size += ((raw_pos[i] - i*group_size)*4);
                }
                org_size = size*2;
            }
#if defined(E_COM) & defined(F_COM)
if ( 1 > 0){
#elif defined(E_COM) 
/*(デバック用) 必ず複合差分圧縮*/
if ( k==0 || (skip==0 && pack_size < org_size)){ 
#elif defined(F_COM) 
if ( k!=0 || (skip==0 && pack_size < org_size)){ 
#else
            if (skip==0 && pack_size < org_size){
#endif
/*(デバック用)必ず非圧縮/ランレングス*/
#if defined(E_NO) | defined(E_RUN)
                if (k==0){goto OUT2;} 
#endif
#if defined(F_NO) | defined(F_RUN) 
                if (k!=0){goto OUT2;} 
#endif

                if (k==0){
/* 指数部は11bitで格納する*/
/*                    for(i=0;i<(group_num-1)*11/8+1;i++){
                        out[off+i]=0;
                    }*/
                    end_p=group_num/8;
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                    for(i=0;i<end_p;i++){
                        out[off+i*11]=group_ref[i*8] >>3;
                        out[off+i*11+1]=(group_ref[i*8] << 5) | ((group_ref[i*8+1] >>6) & 0x1f);
                        out[off+i*11+2]=(group_ref[i*8+1] << 2) | ((group_ref[i*8+2] >>9) & 0x03);
                        out[off+i*11+3]=(group_ref[i*8+2] >> 1);
                        out[off+i*11+4]=(group_ref[i*8+2] << 7) | ((group_ref[i*8+3] >>4) & 0x7f);
                        out[off+i*11+5]=(group_ref[i*8+3] << 4) | ((group_ref[i*8+4] >>7) & 0x0f);
                        out[off+i*11+6]=(group_ref[i*8+4] << 1) | ((group_ref[i*8+5] >>10) & 0x01);
                        out[off+i*11+7]=(group_ref[i*8+5] >> 2 );
                        out[off+i*11+8]=(group_ref[i*8+5] << 6) | ((group_ref[i*8+6] >>5) & 0x3f);
                        out[off+i*11+9]=(group_ref[i*8+6] << 3) | ((group_ref[i*8+7] >>8) & 0x07);
                        out[off+i*11+10]=group_ref[i*8+7];
                    }
                    off += end_p*11;
                    add =0;
/*余り*/
                    if (end_p*8 < group_num){
                        out[off]=group_ref[end_p*8] >>3;
                        out[off+1]=(group_ref[end_p*8] << 5);
                        add =2;
                    }
                    if ((end_p*8+1) < group_num){
                        out[off+1]=out[off+1] | ((group_ref[end_p*8+1] >>6) & 0x1f);
                        out[off+2]=(group_ref[end_p*8+1] << 2);
                        add =3;
                    }
                    if ((end_p*8+2) < group_num){
                        out[off+2]=out[off+2] | ((group_ref[end_p*8+2] >>9) & 0x03);
                        out[off+3]=(group_ref[end_p*8+2] >> 1);
                        out[off+4]=(group_ref[end_p*8+2] << 7);
                        add =5;
                    }
                    if ((end_p*8+3) < group_num){
                        out[off+4]=out[off+4] | ((group_ref[end_p*8+3] >>4) & 0x7f);
                        out[off+5]=(group_ref[end_p*8+3] << 4);
                        add =6;
                    }
                    if ((end_p*8+4) < group_num){
                        out[off+5]=out[off+5] | ((group_ref[end_p*8+4] >>7) & 0x0f);
                        out[off+6]=(group_ref[end_p*8+4] << 1);
                        add =7;
                    }
                    if ((end_p*8+5) < group_num){
                        out[off+6]=out[off+6] | ((group_ref[end_p*8+5] >>10) & 0x01);
                        out[off+7]=(group_ref[end_p*8+5] >> 2 );
                        out[off+8]=(group_ref[end_p*8+5] << 6);
                        add =9;
                    }
                    if ((end_p*8+6) < group_num){
                        out[off+8]=out[off+8] | ((group_ref[end_p*8+6] >>5) & 0x3f);
                        out[off+9]=(group_ref[end_p*8+6] << 3);
                        add =10;
                    }
                    off += add;
                }else{
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                    for (i=0;i<group_num;i++){
                        POKE_N_UI2(out+off+2*i,group_ref[i]);
                    }
                    off +=(2*group_num);
                }
#ifdef DEBUG
                if (debug > 1){
                    printf("group_width offset=%d\n",off);
                }
#endif
#ifdef OMP
#pragma omp parallel for private(i)
#endif
                for (i=0;i<(group_num-1);i=i+2){
                    out[off+i/2]=group_width[i]<< 4 | group_width[i+1];
                }
/*余りビット処理*/
                if ((group_num % 2 ) == 1){
                    out[off+(group_num/2)] = group_width[group_num-1] <<4;
                }
                off += ((group_num-1) / 2 + 1);
#ifdef DEBUG
                if (debug > 1){
                    printf("raw offset=%d %d \n",off,group_num);
                }
#endif
                for (i=0;i<group_num;i++){
/*圧縮されたデータは32bit単位になっているはず*/
                    for (j=i*group_size;j<raw_pos[i];j++){
                        POKE_N_UI4(out+off,raw[j]);
                        off += 4;
                    }
                }
            }else{
                skip =1;
            }
        }
OUT2:
/*ここからが、指数部と仮数部の複合差分圧縮後の処理*/
/*指数部*/
        if (k==0){
            if(skip==0){
/*複合差分圧縮*/
                flag1 = flag1 | 0x10;
            }else if(skip==-1){
/*ランレングス圧縮*/
                end_threads=start_threads;
                while (end_threads < 16){
                    end_threads += threads;
                }
                if (end_threads > 64 ){ end_threads=64;}
#ifdef OMP
#pragma omp parallel for private(i,run_offset)
#endif
                for (i=start_threads;i<end_threads;i++){
                    run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
                    rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,8,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d \n",i,rcs_t[i],(size-1)/8 + 1,byte_adr[i],bit_adr[i]);
                    }
#endif
                }
                for (i=0;i<10;i++){
                    POKE_N_UI4(out+off+i*4,rcs_t[i+6]);
                }
                off += 40;
                POKE_N_UI4(out+off,rcs_t[1]);
                off += 4;
                for (i=0;i<10;i++){
                    memcpy(out+off,runlength_data+(size/4+1)*(i+1),rcs_t[i+6]);
                    off += rcs_t[i+6];
                }
                memcpy(out+off,runlength_data+(size/4+1)*11,rcs_t[1]);
                off += rcs_t[1];
                flag1 = flag1 | 0x20;
                start_threads=end_threads;
            } else {
/*非圧縮*/
#ifdef DEBUG
                if (debug > 0){
                    printf("Not Exec Complex Packing and Spatial Differnecing Schemes k=%d pack_size=%d org_size =%d \n",k,pack_size,org_size);
                }
#endif
                off += nopack_store(buf, out+off, size, 0, 11,8);
                skip =0; /*指数部と仮数部は圧縮傾向が異なるため一旦リセット*/
            }
        }else if (k!=4){
/*最下位4bitを除く仮数部*/
            if(skip==-1){
/*ランレングス圧縮*/
                if (start_threads < (1+k*15)){
                    start_threads =1+k*15;
                }
                end_threads=start_threads;
                while (end_threads < 16+k*15){
                    end_threads += threads;
                }
                if (end_threads > 64){ end_threads=64;}
#ifdef OMP
#pragma omp parallel for private(i,run_offset)
#endif
                for (i=start_threads;i<end_threads;i++){
                    run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
                    rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,8,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d \n",i,rcs_t[i],(size-1)/8 + 1,byte_adr[i],bit_adr[i]);fflush(stdout);
                    }
#endif
                }
                for (i=0;i<15;i++){
                    POKE_N_UI4(out+off+i*4,rcs_t[i+k*15+1]);
                }
                off += 60;
                POKE_N_UI4(out+off,rcs_t[1+k]);
                off += 4;
                for (i=0;i<15;i++){
                    memcpy(out+off,runlength_data+(size/4+1)*(k*16-4+i),rcs_t[i+k*15+1]);
                    off += rcs_t[i+k*15+1];
                }
                memcpy(out+off,runlength_data+(size/4+1)*(k*16+11),rcs_t[1+k]);
                off += rcs_t[1+k];
                start_threads=end_threads;
                if (k==1) {flag1 = flag1 | 0x08;}
                if (k==2) {flag1 = flag1 | 0x02;}
                if (k==3) {flag2 = flag2 | 0x80;}
            }else if (skip==1){
/*非圧縮*/
#ifdef DEBUG
                if (debug > 0){
                    printf("Not Exec Complex Packing and Spatial Differnecing Schemes k= %d pack_size=%d org_size =%d \n",k,pack_size,org_size);
                }
#endif
                off = off_old;
                off += nopack_store(buf, out+off, size, k*2, 16,8);
            }else{ 
/*複合差分圧縮*/
                if (k==1) {flag1 = flag1 | 0x04;}
                if (k==2) {flag1 = flag1 | 0x01;}
                if (k==3) {flag2 = flag2 | 0x40;}
            }
        }else{
/*最下位4bitの仮数部*/
            if(skip==-1){
/*ランレングス圧縮*/
                if (start_threads < 61){
                    start_threads =61;
                }
                end_threads=64;
#ifdef OMP
#pragma omp parallel for private(i,run_offset)
#endif
                for (i=start_threads;i<end_threads;i++){
                    run_offset = (byte_adr[i]*8+bit_adr[i])*(size/4+1);
                    rcs_t[i]=encode_runlength(buf,runlength_data+run_offset,size,8,byte_adr[i],bit_adr[i]);
#ifdef DEBUG
                    if (debug > 0){
                        printf("RunLength Encooding(%d) pack_size=%d org_size =%d %d %d\n",i,rcs_t[i],(size-1)/8 + 1,byte_adr[i],bit_adr[i]);
                    }
#endif
                }
                for (i=0;i<3;i++){
                    POKE_N_UI4(out+off+i*4,rcs_t[i+61]);
                }
                off += 12;
                POKE_N_UI4(out+off,rcs_t[5]);
                off += 4;
                for (i=0;i<3;i++){
                    memcpy(out+off,runlength_data+(size/4+1)*(i+60),rcs_t[i+61]);
                    off += rcs_t[i+61];
                }
                memcpy(out+off,runlength_data+(size/4+1)*63,rcs_t[5]);
                off += rcs_t[5];
                flag2 = flag2 | 0x20;
            }else if (skip==1 ){
/*非圧縮*/
#ifdef DEBUG
                if (debug > 0){
                    printf("Not Exec Complex Packing and Spatial Differnecing Schemes k= %d pack_size=%d org_size =%d %d %d\n",k,pack_size,org_size,off,off_old);
                }
#endif
                off = off_old;
                off += nopack_store(buf, out+off, size, 0, 4,8);
            }else{
/*複合差分圧縮*/
                flag2 = flag2 | 0x10;
            }
        }
#ifdef DEBUG
        if (debug > 0){
            int bits ;
            if (k==0){ bits=11;
            }else if(k==1){ bits=16;
            }else if(k==2){ bits=16;
            }else if(k==3){ bits=16;
            }else{bits=4;
            }
            if (debug > 0){
                printf("Part %d Packing_size=%d Org_size=%d Rate =%5.2f(%%) offset=%d skip=%d\n",k, off-off_old,(size*bits-1)/8,(float)(off-off_old)/(size*bits-1)*8*100.0,off,skip);
            }
        }
#endif
    }
#ifdef DEBUG
    if (debug > 0){
        printf("Total Packing_size=%d Org_size=%d Rate =%f(%%) \n",off,size*8,(float)off/size/8*100.0);fflush(stdout);
    }
#endif

/*最後にデータ長とフラグを書き込む*/
    POKE_N_SI4(out+4,off);
    memcpy(out+8,&flag1,1);
    memcpy(out+9,&flag2,1);
    free(group_width);
    free(group_ref);
    free(pack2);
    free(unpack);
    free(raw);
    free(raw_pos);
    free(runlength_data);
    if (little_endian ==1){
        free(buf);
    }
    return (off);
}
int encode_r8a(unsigned  char *org, unsigned  char *out, int size){
/* #ifdef Little_Endian -- nusdasのconfigureで元々存在したBIGENDIAN判定configを使うように変更*/
#ifndef WORDS_BIGENDIAN
    return (encode_r8(org,out,size,1));
#else
    return (encode_r8(org,out,size,0));
#endif
}
int encode_r4a(unsigned  char *org, unsigned  char *out, int size){
/* #ifdef Little_Endian -- nusdasのconfigureで元々存在したBIGENDIAN判定configを使うように変更*/
#ifndef WORDS_BIGENDIAN
    return (encode_r4(org,out,size,1));
#else
    return (encode_r4(org,out,size,0));
#endif
}
int decode_r8a(unsigned  char *org, unsigned  char *out){
/* #ifdef Little_Endian -- nusdasのconfigureで元々存在したBIGENDIAN判定configを使うように変更*/
#ifndef WORDS_BIGENDIAN
    return (decode_r8(org,out,1));
#else
    return (decode_r8(org,out,0));
#endif
}
int decode_r4a(unsigned  char *org, unsigned  char *out){
/* #ifdef Little_Endian -- nusdasのconfigureで元々存在したBIGENDIAN判定configを使うように変更*/
#ifndef WORDS_BIGENDIAN
    return (decode_r4(org,out,1));
#else
    return (decode_r4(org,out,0));
#endif
}
/*
fortran I/F
*/
void ENCODE_R8(unsigned  char *org,unsigned  char *out,int *size, int *mode ,int *rc){
    *rc=encode_r8(org,out,*size,*mode);
}

void DECODE_R8(unsigned  char *org,unsigned char *out,int *mode ,int *rc){
    *rc=decode_r8(org,out,*mode);
}

void ENCODE_R4(unsigned  char *org,unsigned  char *out,int *size, int *mode ,int *rc){
    *rc=encode_r4(org,out,*size,*mode);
}

void DECODE_R4(unsigned  char *org,unsigned char *out,int *mode ,int *rc){
    *rc=decode_r4(org,out,*mode);
}

void encode_r8_(unsigned  char *org,unsigned  char *out,int *size, int *mode ,int *rc){
    *rc=encode_r8(org,out,*size,*mode);
}
void decode_r8_(unsigned  char *org,unsigned char *out,int *mode ,int *rc){
    *rc=decode_r8(org,out,*mode);
}

void encode_r4_(unsigned  char *org,unsigned  char *out,int *size, int *mode ,int *rc){
    *rc=encode_r4(org,out,*size,*mode);
}

void decode_r4_(unsigned  char *org,unsigned char *out,int *mode ,int *rc){
    *rc=decode_r4(org,out,*mode);
}
void ENCODE_R8A(unsigned  char *org,unsigned  char *out,int *size, int *rc){
    *rc=encode_r8a(org,out,*size);
}

void DECODE_R8A(unsigned  char *org,unsigned char *out,int *rc){
    *rc=decode_r8a(org,out);
}

void ENCODE_R4A(unsigned  char *org,unsigned  char *out,int *size,int *rc){
    *rc=encode_r4a(org,out,*size);
}

void DECODE_R4A(unsigned  char *org,unsigned char *out,int *rc){
    *rc=decode_r4a(org,out);
}

void encode_r8a_(unsigned  char *org,unsigned  char *out,int *size,int *rc){
    *rc=encode_r8a(org,out,*size);
}
void decode_r8a_(unsigned  char *org,unsigned char *out,int *rc){
    *rc=decode_r8a(org,out);
}

void encode_r4a_(unsigned  char *org,unsigned  char *out,int *size,int *rc){
    *rc=encode_r4a(org,out,*size);
}

void decode_r4a_(unsigned  char *org,unsigned char *out,int *rc){
    *rc=decode_r4a(org,out);
}
#ifdef Test
int main(int argc, char *argv[]){
    FILE *fp,*fp2;
    int x,y;
    unsigned char *buf,*out,*buf2;
    int rc;
    int count =0;
    long long total_read=0;
    long long total_pack=0;
    double d1;
    int i;
    int size;
    struct timeval t1,t2;
    float decode_t=0;
    float encode_t=0;

/* debug */
    debug =1;

    if (argc <4){
        printf("pgm input x_size y_size\n");
        exit (0);
    }
    x=atoi(argv[2]);
    y=atoi(argv[3]);
    if (x < 1 || y < 1 ){
        printf("x=%d y=%d size error\n",x,y);
        exit(1);
    }
    if ((fp=fopen(argv[1],"r"))==NULL){
        printf("%s open error\n",argv[1]);
        exit(1);
    }
/*
   本来は元サイズ+100バイトで良いが、強制ランレングスモードの場合元サイズの2倍の容量が必要 
*/
    if ((buf=malloc(x*y*8*2+100)) == NULL){
        printf("malloc error(buf) size=%d\n",x*y*8);
        exit(1);
    }
    if ((buf2=malloc(x*y*8*2+100)) == NULL){
        printf("malloc error(buf2) size=%d\n",x*y*8);
        exit(1);
    }
    if ((out=malloc(x*y*8*2+100)) == NULL){
        printf("malloc error(out) size=%d\n",x*y*8);
        exit(1);
    }
    printf("Input =%s x=%d y=%d \n",argv[1],x,y);
#if defined(MAKE_DUMMY)
/* 自前でテストデータを作る時はここを有効にする*/
    for (i=0;i<x*y;i++){
        d1 = i*0.1 +0.1;
        memcpy(buf+i*8,&d1,8);
    }
#else
    if ((fp=fopen(argv[1],"r"))==NULL){
        printf("%s open error\n",argv[1]);
        exit(1);
    }
#endif
#if defined(READ_PACK)
    if ((fp2=fopen("UNPACK","w"))==NULL){
        printf("%s open error\n",argv[1]);
        exit(1);
    }
    while (fread (buf,1,8,fp) == 8){
        size=PEEK_N_UI4(buf+4);
        printf("read size=%d\n",size);
        fread(buf+8,1,size-8,fp);
        rc=decode_r8(buf,out,0);
printf("unpacking %d %d %d\n",rc,size);
        if (fwrite(out,1,rc*8,fp2) != rc*8 ){
/*        if (fwrite(buf,1,x*y*8,fp2) != x*y*8 ){*/
            printf("write error size=%d\n",rc);
            exit(1);
        }
    }
#endif
#if defined(READ_RAW)
    if ((fp2=fopen("PACK","w"))==NULL){
        printf("%s open error\n",argv[1]);
        exit(1);
    }
#if defined(R4)
    while (fread (buf,1,4,fp) == 4){
        if (fread(buf,1,x*y*4,fp) != (x*y*4) ){
            break;
        } 
        total_read += x*y*4;
        count ++;

        gettimeofday(&t1,NULL);
        rc=encode_r4(buf,out,x*y,0);
        gettimeofday(&t2,NULL);
        encode_t += (t2.tv_usec - t1.tv_usec)/1000000.0 + (t2.tv_sec - t1.tv_sec); 
        total_pack += rc;
printf("packing %d %d %d\n",count,rc,x*y*4);fflush(stdout);
        if (fwrite(out,1,rc,fp2) != rc ){
/*        if (fwrite(buf,1,x*y*8,fp2) != x*y*8 ){*/
            printf("write error size=%d\n",rc);
            exit(1);
        }

#if defined(READ_DECODE)
        gettimeofday(&t1,NULL);
        rc=decode_r4(out, buf2,0);
        gettimeofday(&t2,NULL);
        decode_t += (t2.tv_usec - t1.tv_usec)/1000000.0 + (t2.tv_sec - t1.tv_sec); 

        printf("unpacking %d %d %d\n",count,rc,x*y*4);fflush(stdout);
        for(i=0;i<x*y*4;i++){
            if(buf[i]!=buf2[i]){
                printf("X %d %x %x\n",i,buf[i],buf2[i]);
                exit(1);
            }
        }
#endif
        if (fread(buf,1,4,fp) != 4 ){
            break;
        }

    }
#else
    while (fread (buf,1,4,fp) == 4){
        if (fread(buf,1,x*y*8,fp) != (x*y*8) ){
            break;
        } 
        total_read += x*y*8;
        count ++;

        gettimeofday(&t1,NULL);
        rc=encode_r8(buf,out,x*y,0);
        gettimeofday(&t2,NULL);
        encode_t += (t2.tv_usec - t1.tv_usec)/1000000.0 + (t2.tv_sec - t1.tv_sec); 
        total_pack += rc;
printf("packing %d %d %d\n",count,rc,x*y*8);fflush(stdout);
        if (fwrite(out,1,rc,fp2) != rc ){
/*        if (fwrite(buf,1,x*y*8,fp2) != x*y*8 ){*/
            printf("write error size=%d\n",rc);
            exit(1);
        }

#if defined(READ_DECODE)
        gettimeofday(&t1,NULL);
        rc=decode_r8(out, buf2,0);
        gettimeofday(&t2,NULL);
        decode_t += (t2.tv_usec - t1.tv_usec)/1000000.0 + (t2.tv_sec - t1.tv_sec); 

        printf("unpacking %d %d %d\n",count,rc,x*y*8);fflush(stdout);
        for(i=0;i<x*y*8;i++){
            if(buf[i]!=buf2[i]){
                printf("X %d %x %x\n",i,buf[i],buf2[i]);
                exit(1);
            }
        }
#endif
        if (fread(buf,1,4,fp) != 4 ){
            break;
        }

    }
#endif
#endif
    printf("ALL TOTAL PACK=%lld org=%lld rate=%5.2f(%%)\n",total_pack,total_read,(float) total_pack/total_read*100.0);
    printf("encode t =%7.3f\n",encode_t);
    printf("decode t =%7.3f\n",decode_t);
    fclose(fp);
    fclose(fp2);
    free(buf);
    return (0);
}
#endif
