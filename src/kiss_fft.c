#define _USE_MATH_DEFINES
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct {
    float r;
    float i;
} kiss_fft_cpx;

typedef struct kiss_fft_state{
    int nfft;
    int inverse;
    int factors[2*32];
    kiss_fft_cpx twiddles[1];
} *kiss_fft_cfg;

#define MAXFACTORS 32

#define S_MUL(a,b) ((a)*(b))

#define C_MUL(m,a,b) \
    do { (m).r = (a).r*(b).r - (a).i*(b).i; \
         (m).i = (a).r*(b).i + (a).i*(b).r; } while(0)

#define C_FIXDIV(c,div) /* nothing */
#define C_ADD(res,a,b) (res).r=(a).r+(b).r; (res).i=(a).i+(b).i
#define C_SUB(res,a,b) (res).r=(a).r-(b).r; (res).i=(a).i-(b).i
#define C_ADDTO(res,a) (res).r += (a).r; (res).i += (a).i
#define C_SUBFROM(res,a) (res).r -= (a).r; (res).i -= (a).i

#define KISS_FFT_COS(phase) (float)cos(phase)
#define KISS_FFT_SIN(phase) (float)sin(phase)
#define HALF_OF(x) ((x)*0.5f)

#define kf_cexp(x,phase) \
    do { (x)->r = KISS_FFT_COS(phase); \
         (x)->i = KISS_FFT_SIN(phase); } while(0)

static void kf_bfly2(kiss_fft_cpx * Fout, const size_t fstride, const kiss_fft_cfg st, int m) {
    kiss_fft_cpx * Fout2;
    kiss_fft_cpx * tw1 = st->twiddles;
    kiss_fft_cpx t;
    Fout2 = Fout + m;
    do {
        C_FIXDIV(*Fout,2); C_FIXDIV(*Fout2,2);
        C_MUL( t,  *Fout2 , *tw1 );
        C_SUB( *Fout2 ,  *Fout , t );
        C_ADDTO( *Fout ,  t );
        ++Fout2;
        ++Fout;
        ++tw1;
    } while (--m);
}

static void kf_bfly4(kiss_fft_cpx * Fout, const size_t fstride, const kiss_fft_cfg st, const size_t m) {
    kiss_fft_cpx *tw1,*tw2,*tw3;
    kiss_fft_cpx scratch[6];
    size_t k=m;
    const size_t m2=2*m;
    const size_t m3=3*m;

    tw3 = tw2 = tw1 = st->twiddles;

    do {
        C_FIXDIV(*Fout,4); C_FIXDIV(Fout[m],4); C_FIXDIV(Fout[m2],4); C_FIXDIV(Fout[m3],4);

        C_MUL(scratch[0],Fout[m] , *tw1 );
        C_MUL(scratch[1],Fout[m2] , *tw2 );
        C_MUL(scratch[2],Fout[m3] , *tw3 );

        C_SUB( scratch[5] , *Fout, scratch[1] );
        C_ADDTO( *Fout , scratch[1] );
        C_ADD( scratch[3] , scratch[0] , scratch[2] );
        C_SUB( scratch[4] , scratch[0] , scratch[2] );
        C_SUB( Fout[m2], *Fout, scratch[3] );
        C_ADDTO( *Fout , scratch[3] );
        C_MUL(scratch[0] , scratch[5] , st->twiddles[fstride*m] );
        C_MUL(scratch[1] , scratch[4] , st->twiddles[fstride*m2] );
        C_ADD( Fout[m] , scratch[0] , scratch[1] );
        C_SUB( Fout[m3] , scratch[0] , scratch[1] );
        ++Fout;
    } while(--k);
}

static void kf_bfly3(kiss_fft_cpx * Fout, const size_t fstride, const kiss_fft_cfg st, size_t m) {
    size_t k=m;
    const size_t m2 = 2*m;
    kiss_fft_cpx *tw1,*tw2;
    kiss_fft_cpx scratch[5];
    kiss_fft_cpx epi3;
    epi3 = st->twiddles[fstride*m];

    tw1=tw2=st->twiddles;

    do {
        C_FIXDIV(*Fout,3); C_FIXDIV(Fout[m],3); C_FIXDIV(Fout[m2],3);

        C_MUL(scratch[1],Fout[m] , *tw1);
        C_MUL(scratch[2],Fout[m2] , *tw2);

        C_ADD(scratch[3],scratch[1],scratch[2]);
        C_SUB(scratch[0],scratch[1],scratch[2]);
        tw1 += fstride;
        tw2 += fstride*2;

        Fout[m].r = Fout->r - HALF_OF(scratch[3].r);
        Fout[m].i = Fout->i - HALF_OF(scratch[3].i);

        C_MUL(scratch[0], scratch[0] , epi3);

        C_ADDTO(*Fout,scratch[3]);

        Fout[m2].r = Fout[m].r + scratch[0].r;
        Fout[m2].i = Fout[m].i + scratch[0].i;

        C_SUB(Fout[m].r , Fout[m].r , scratch[0].r);
        C_SUB(Fout[m].i , Fout[m].i , scratch[0].i);

        ++Fout;
    } while(--k);
}

static void kf_bfly5(kiss_fft_cpx * Fout, const size_t fstride, const kiss_fft_cfg st, int m) {
    kiss_fft_cpx *Fout0,*Fout1,*Fout2,*Fout3,*Fout4;
    int u;
    kiss_fft_cpx scratch[13];
    kiss_fft_cpx * twiddles = st->twiddles;
    kiss_fft_cpx *tw;
    kiss_fft_cpx ya,yb;
    ya = twiddles[fstride*m];
    yb = twiddles[fstride*2*m];

    Fout0=Fout;
    Fout1=Fout0+m;
    Fout2=Fout0+2*m;
    Fout3=Fout0+3*m;
    Fout4=Fout0+4*m;

    tw=st->twiddles;
    for ( u=0; u<m; ++u ) {
        C_FIXDIV( *Fout0,5); C_FIXDIV( *Fout1,5); C_FIXDIV( *Fout2,5); C_FIXDIV( *Fout3,5); C_FIXDIV( *Fout4,5);
        scratch[0] = *Fout0;

        C_MUL(scratch[1] , *Fout1 , tw[u*fstride]);
        C_MUL(scratch[2] , *Fout2 , tw[2*u*fstride]);
        C_MUL(scratch[3] , *Fout3 , tw[3*u*fstride]);
        C_MUL(scratch[4] , *Fout4 , tw[4*u*fstride]);

        C_ADD( scratch[7],scratch[1],scratch[4]);
        C_SUB( scratch[10],scratch[1],scratch[4]);
        C_ADD( scratch[8],scratch[2],scratch[3]);
        C_SUB( scratch[9],scratch[2],scratch[3]);

        Fout0->r += scratch[7].r + scratch[8].r;
        Fout0->i += scratch[7].i + scratch[8].i;

        scratch[5].r = scratch[0].r + S_MUL(scratch[7].r,ya.r) + S_MUL(scratch[8].r,yb.r);
        scratch[5].i = scratch[0].i + S_MUL(scratch[7].i,ya.r) + S_MUL(scratch[8].i,yb.r);

        scratch[6].r =  S_MUL(scratch[10].i,ya.i) + S_MUL(scratch[9].i,yb.i);
        scratch[6].i = -S_MUL(scratch[10].r,ya.i) - S_MUL(scratch[9].r,yb.i);

        C_SUB(*Fout1,scratch[5],scratch[6]);
        C_ADD(*Fout4,scratch[5],scratch[6]);

        scratch[11].r = scratch[0].r + S_MUL(scratch[7].r,yb.r) + S_MUL(scratch[8].r,ya.r);
        scratch[11].i = scratch[0].i + S_MUL(scratch[7].i,yb.r) + S_MUL(scratch[8].i,ya.r);
        scratch[12].r = - S_MUL(scratch[10].i,yb.i) + S_MUL(scratch[9].i,ya.i);
        scratch[12].i = S_MUL(scratch[10].r,yb.i) - S_MUL(scratch[9].r,ya.i);

        C_ADD(*Fout2,scratch[11],scratch[12]);
        C_SUB(*Fout3,scratch[11],scratch[12]);

        ++Fout0;++Fout1;++Fout2;++Fout3;++Fout4;
    }
}

static void kf_work(kiss_fft_cpx * Fout, const kiss_fft_cpx * f, const size_t fstride, int in_stride, int * factors, const kiss_fft_cfg st) {
    const int p=*factors++;
    const int m=*factors++;
    const kiss_fft_cpx * Fout_end = Fout + p*m;

    if (p==2)
        kf_bfly2(Fout, fstride, st, m);
    else if (p==3)
        kf_bfly3(Fout, fstride, st, m);
    else if (p==4)
        kf_bfly4(Fout, fstride, st, m);
    else if (p==5)
        kf_bfly5(Fout, fstride, st, m);
}

static void kf_factor(int n,int * facbuf) {
    int p=4;
    double floor_sqrt = floor( sqrt((double)n) );
    while (n > 1) {
        while (n % p) {
            switch (p) {
                case 4: p = 2; break;
                case 2: p = 3; break;
                default: p += 2; break;
            }
            if (p > floor_sqrt) p = n;
        }
        n /= p;
        *facbuf++ = p;
        *facbuf++ = n;
    }
}

kiss_fft_cfg kiss_fft_alloc(int nfft,int inverse_fft,void * mem,size_t * lenmem ) {
    kiss_fft_cfg st = NULL;
    size_t memneeded = sizeof(struct kiss_fft_state) + sizeof(kiss_fft_cpx)*(nfft-1);
    if (lenmem==NULL) {
        st = (kiss_fft_cfg) malloc(memneeded);
    } else {
        if (mem != NULL && *lenmem >= memneeded)
            st = (kiss_fft_cfg)mem;
        *lenmem = memneeded;
    }
    if (st) {
        int i;
        st->nfft=nfft;
        st->inverse = inverse_fft;
        for (i=0;i<nfft;++i) {
            double phase = -2*M_PI*i / nfft;
            if (st->inverse)
                phase *= -1;
            kf_cexp(st->twiddles+i, phase );
        }
        kf_factor(nfft,st->factors);
    }
    return st;
}

void kiss_fft(kiss_fft_cfg cfg,const kiss_fft_cpx *fin,kiss_fft_cpx *fout) {
    int i;
    const int nfft = cfg->nfft;
    const int inverse = cfg->inverse;
    int factors[2*MAXFACTORS];
    memcpy(factors,cfg->factors,sizeof(int)*(cfg->factors[0]*2+2) );

    kf_work( fout, fin, 1, 1, factors, cfg );

    if (inverse) {
        double scaling = 1.0/nfft;
        for (i=0;i<nfft;++i) {
            fout[i].r *= scaling;
            fout[i].i *= scaling;
        }
    }
}