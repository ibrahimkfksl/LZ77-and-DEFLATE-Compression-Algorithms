#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>



///////////////////////////////
//Huffman icin gerekli tanimlamalar

typedef struct huffman
{
    char c;
    int frek;
    struct huffman *sol;
    struct huffman *sag;
} HUFFMANAGAC;

typedef struct huffman_dizisi {
    int kapasite;
    int boyut;
    HUFFMANAGAC **nesne;
} HUFFMANDIZISI;


///////////////////
/////////////////////////////////
// LZ77 icin Gerekli Tanimlamalar
int metin_boyutu=0;
char *metin;
int lz77_cikti_boyut=0;

int maksBenzerlik=0;
int baslangicIndex=0;
char son_harf;

void dosyaOku();
void lz77();
void benzerlik_bul(int);


// Deflate icin Gerekli Tanimlamalar
int boyutLZSS=0;//LZSS ciktisinin boyutu
char *metinLZSS;//LZSS Stringi
char huffman_karakter[100];//Huffman a ekledigimiz karakterleri burada da tutuyoruz
char *huffman_bityolu[100];//Huffman a ekledigimiz karakterin agacta bulunan veri yolunu string olarak burada tutuyoruz
int huffman_tablosu_boyutu;//Yukaridaki bityolu ve karakter tablosunun boyutlarini tutuyoruz
int boyutDeflate=0;

void deflate();//Deflate algoritmasi icin calismasi gereken fonksiyonlari calistiriyor
void dosyaOku2();//LZSS Dosyasini bellege aktariyor
void huffmanAgacOlustur();//Huffman agacini olusturmak icin gerekli fonksiyonlari calistiryor
char* veriYolBul(char *c);//Fonksiyona gonderilen karakterin agactaki yolunu geri donderir

void calistir();//Kodlama Programini Baslatir
void sonuclariKarsilastir();//Kodlanmis Hallerin Boyutlarini Ekrana Basar

///////////////////////////////////
HUFFMANAGAC *yeniHuffmanAgaci(char c, int frek);
void huffmanVeriYolunuKayitEt(HUFFMANAGAC *t, char *prefix, int boyut_prefix);
HUFFMANDIZISI *yeniHuffmanDizisi();
void *huffmanDizisiEkle(HUFFMANDIZISI *dizi, HUFFMANAGAC *t);
HUFFMANAGAC *huffmanDizisiPop(HUFFMANDIZISI *dizi);//Dizinin son elemanini cikarir
void huffmanDiziSiralama(HUFFMANDIZISI *dizi);
//////////////////////////////////////////////////////


int main()
{
    calistir();
    return 0;
}


void calistir(){
    printf("metin.txt dosyasinin kodlama islemi basladi lutfen bekleyiniz...\n\n\n");
    dosyaOku();
    lz77();
    printf("\n\n\n");
    deflate();
    sonuclariKarsilastir();

    printf("\n\n\n");
    printf("Programi Kapatmak Icin Bir Tusa Tiklayiniz\n");
    getchar();
    exit(1);

}
void sonuclariKarsilastir(){

    FILE *dosya;
    dosya = fopen("metin.txt", "rb");
    fseek(dosya,0,SEEK_END);
    metin_boyutu=ftell(dosya);
    fclose(dosya);

    dosya = fopen("lz77Cikti.txt", "rb");
    fseek(dosya,0,SEEK_END);
    lz77_cikti_boyut=ftell(dosya);
    fclose(dosya);

    dosya = fopen("DeflateSonuc.txt", "rb");
    fseek(dosya,0,SEEK_END);
    boyutDeflate=ftell(dosya);
    fclose(dosya);
    printf("\n\nSIKISTIRMA CIKTILARI ILGILI .txt DOSYALARINA KAYIT EDILDI\n\n");
    printf("\n\n------------Boyutlar-----------------\n\n");
    printf("Orijinal Boyut : %d bytes\n",metin_boyutu);
    printf("Lz77 ile Kodlanmis Boyut: %d bytes\n",lz77_cikti_boyut);
    printf("Deflate ile Kodlanmis Boyut: %d bytes\n",(boyutDeflate));


}

void deflate()
{
    dosyaOku2();
    huffmanAgacOlustur();
    huffmanUygula();

}

void lz77()
{
    FILE *fp;
    if((fp = fopen("lz77Cikti.txt","w"))==NULL)
    {
        printf("lz77 Dosya açma hatasi!");
        exit(1);
    }

    FILE *fp2;
    if((fp2 = fopen("lzSSCikti.txt","wb"))==NULL)
    {
        printf("lzSSCikt Dosya acma hatasi!");
        exit(1);
    }

    printf("--------------------LZ77 CIKTISI--------------------\n");
    for(int i=0; i<metin_boyutu+1; i++)
    {
        benzerlik_bul(i);
        if(maksBenzerlik==0)
        {
            printf("<0,0,%c>",son_harf);
            fprintf(fp,"0%c",son_harf);
            fprintf(fp2,"%c",son_harf);
        }
        else
        {
            printf("<%d,%d,%c>",i-baslangicIndex,maksBenzerlik,son_harf);
            fprintf(fp,"%d|%d%c",i-baslangicIndex,maksBenzerlik,son_harf);
            fprintf(fp2,"%d%d%c",i-baslangicIndex,maksBenzerlik,son_harf);

        }
        i+=maksBenzerlik;
    }

        fclose(fp);
        fclose(fp2);
}


void benzerlik_bul(int konum)
{
    int benzerlik=0;
    int baslangic_noktasi;
    int index=0;
    int kontrol=0;
    maksBenzerlik=0;
    baslangicIndex=0;

    if(konum<482)
    {
        baslangic_noktasi=0;
    }
    else
    {
        baslangic_noktasi=konum-481;
    }

    int bitis_noktasi=konum;
    son_harf=metin[konum];

    for(int j=baslangic_noktasi; j<bitis_noktasi; j++)
    {

       if(konum==metin_boyutu)
        {
            break;
        }
        if(metin[j]==metin[konum])
        {

            if(kontrol==0)
            {
                index=j;
            }
            benzerlik++;
            kontrol=1;
            konum++;

        }
        else
        {
            kontrol=0;
            if(maksBenzerlik<benzerlik)
            {
                maksBenzerlik=benzerlik;
                baslangicIndex=index;
                son_harf=metin[konum];
            }
            index=0;
            benzerlik=0;
            konum=bitis_noktasi;
        }
        if(benzerlik>=29)
        {
            if(maksBenzerlik<benzerlik)
            {
                maksBenzerlik=benzerlik;
                baslangicIndex=index;
                son_harf=metin[konum];
                konum=bitis_noktasi;

            }
            break;
        }
        if(j==bitis_noktasi-1)
        {
            kontrol=0;
            if(maksBenzerlik<benzerlik)
            {
                maksBenzerlik=benzerlik;
                baslangicIndex=index;
                son_harf=metin[konum];
            }
            index=0;
            benzerlik=0;
            konum=bitis_noktasi;
            break;
        }

    }
}

void dosyaOku()
{
    int i=0;
    metin = malloc(sizeof(char));
    FILE *fp;

    if((fp = fopen("metin.txt","r"))==NULL)
    {
        printf("Dosya açma hatasi!");
        exit(1);
    }

    while(!feof(fp))
    {
        metin[i]=fgetc(fp);
        if(ferror(fp))
        {
                printf("Dosyadan okuma hatasi!");
                exit(1);
        }
        if(metin[i]!='\n')
        {
            i++;
            metin = realloc(metin,sizeof(char)*(i+1));
        }
        else
        {
            metin[i]=' ';
            i++;
            metin = realloc(metin,sizeof(char)*(i+1));
        }
    }

    metin_boyutu=i-2;
    // free(metin);
}

void dosyaOku2()
{
    int i=0;
    metinLZSS = malloc(sizeof(char));
    FILE *fp;

    if((fp = fopen("lzSSCikti.txt","r"))==NULL)
    {
        printf("lzSSCikti.txt Dosyasi okunma hatasi!");
        exit(1);
    }

    while(!feof(fp))
    {
        metinLZSS[i]=fgetc(fp);
        if(ferror(fp))
        {
                printf("Dosyadan okuma hatasý!");
                exit(1);
        }
        if(metinLZSS[i]!='\n')
        {
            i++;
            metinLZSS = realloc(metinLZSS,sizeof(char)*(i+1));
        }
        else
        {
            metinLZSS[i]=' ';
            i++;
            metinLZSS = realloc(metinLZSS,sizeof(char)*(i+1));
        }
    }

    boyutLZSS=i-2;
}


//////////////////////////////////////////////////

void huffmanAgacOlustur(){
    unsigned long frekans[0xFF];
    char *pcTemp;
    int i;
    HUFFMANDIZISI *dizi = yeniHuffmanDizisi();

    memset(&frekans[0], 0, sizeof(frekans));

    for (pcTemp = &metinLZSS[0]; *pcTemp != 0; pcTemp++)
    {
        frekans[(int)*pcTemp]++;

    }

    for (i = 0; i < 255; i++)
    {
        if (frekans[i] > 0)
        {
            huffmanDizisiEkle(dizi, yeniHuffmanAgaci(i, frekans[i]));
        }
    }

    while (dizi->boyut > 1)
    {
        huffmanDiziSiralama(dizi);
        HUFFMANAGAC *t1 = huffmanDizisiPop(dizi);
        HUFFMANAGAC *t2 = huffmanDizisiPop(dizi);
        HUFFMANAGAC *t3 = calloc(1, sizeof(HUFFMANAGAC));
        t3->sol = t1;
        t3->sag = t2;
        t3->frek = t1->frek + t2->frek;
        huffmanDizisiEkle(dizi, t3);

    }




    for( i=0;i<100;i++){
        huffman_karakter[i]=NULL;
        huffman_bityolu[i]=malloc(256);
    }
    huffman_tablosu_boyutu=0;


    char *buffer = malloc(256);
    huffmanVeriYolunuKayitEt(dizi->nesne[0], buffer, 0);

}


HUFFMANAGAC *yeniHuffmanAgaci(char c, int frek)
{
    HUFFMANAGAC *t = malloc(sizeof(HUFFMANAGAC));
    t->c = c;
    t->frek = frek;
    t->sol = NULL;
    t->sag = NULL;
    return t;
}


void huffmanDiziSiralama(HUFFMANDIZISI *dizi)
{//buyukten kucuge selection sort yapiyor
    int i, k;
    int max_index, max_value;

    for (i = 0; i < dizi->boyut - 1; i++)
    {
        max_index = i;
        max_value = dizi->nesne[i]->frek;

        for (k = i + 1; k < dizi->boyut; k++)
        {
            if (dizi->nesne[k]->frek > max_value)
            {
                max_value = dizi->nesne[k]->frek;
                max_index = k;
            }
        }

        if (i != max_index)
        {

            HUFFMANAGAC *tmp = malloc(sizeof(HUFFMANAGAC));
            tmp=dizi->nesne[i];
            dizi->nesne[i]=dizi->nesne[max_index];
            dizi->nesne[max_index]=tmp;
            tmp=NULL;
            free(tmp);
        }

    }
}

HUFFMANAGAC *huffmanDizisiPop(HUFFMANDIZISI *dizi)
{
    return dizi->nesne[--dizi->boyut];
}

void *huffmanDizisiEkle(HUFFMANDIZISI *dizi, HUFFMANAGAC *t)
{
    if (dizi->boyut + 1 == dizi->kapasite)
    {
        dizi->kapasite *= 2;
        dizi->nesne = realloc(dizi->nesne, dizi->kapasite * sizeof(HUFFMANAGAC *));
    }

    dizi->nesne[dizi->boyut++] = t;
}

HUFFMANDIZISI *yeniHuffmanDizisi()
{
    HUFFMANDIZISI *dizi = malloc(sizeof(HUFFMANDIZISI));
    dizi->kapasite = 8;
    dizi->boyut = 0;
    dizi->nesne = malloc(dizi->kapasite * sizeof(HUFFMANAGAC *) );
    return dizi;
}

void huffmanVeriYolunuKayitEt(HUFFMANAGAC *t, char *prefix, int boyut_prefix)
{
    if (t->sol == NULL && t->sag == NULL)
    {
        prefix[boyut_prefix] = 0;

        huffman_karakter[huffman_tablosu_boyutu]=t->c;
        strcpy(huffman_bityolu[huffman_tablosu_boyutu++],prefix);

        return;
    }

    if (t->sol)
    {
        prefix[boyut_prefix++] = '0';
        huffmanVeriYolunuKayitEt(t->sol, prefix, boyut_prefix);
        boyut_prefix--;
    }

    if (t->sag)
    {
        prefix[boyut_prefix++] = '1';
        huffmanVeriYolunuKayitEt(t->sag, prefix, boyut_prefix);
        boyut_prefix--;
    }

}

char* veriYolBul(char *c){
    int i;
    for(i=0;i<huffman_tablosu_boyutu;i++){
        if(c==huffman_karakter[i]){
            return huffman_bityolu[i];
        }
    }
}

void huffmanUygula(){

    char *karakterin_veri_yolu=malloc(256);
    int i,j;
    int huffman_bit_kontrol=0;//7 den geri dogru saymak icin
    char huffman_sonucu=0;

    FILE *fp;
    if((fp = fopen("DeflateSonuc.txt","w"))==NULL)
    {
        printf("DeflateSonuc Dosya açma hatasi!");
        exit(1);
    }

    for(i=0;i<boyutLZSS;i++){
        karakterin_veri_yolu=veriYolBul(metinLZSS[i]);


        for(j=0;karakterin_veri_yolu[j]!='\0';j++){
            boyutDeflate++;

            if(karakterin_veri_yolu[j]=='1'){
                char x=1<<(7-huffman_bit_kontrol);
                huffman_sonucu|=x;
                huffman_bit_kontrol++;
                if(huffman_bit_kontrol==8){
                    huffman_bit_kontrol=0;
                    fprintf(fp,"%c",huffman_sonucu);
                    huffman_sonucu=0;
                }
            }
            else if(karakterin_veri_yolu[j]=='0'){
                char x=0<<(7-huffman_bit_kontrol);
                huffman_sonucu|=x;
                huffman_bit_kontrol++;
                if(huffman_bit_kontrol==8){
                    huffman_bit_kontrol=0;
                    fprintf(fp,"%c",huffman_sonucu);
                    huffman_sonucu=0;
                }
            }
        }




    }
    rewind(fp);
    fprintf(fp,"%d,",huffman_bit_kontrol);
    fclose(fp);
}
