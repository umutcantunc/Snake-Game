#include "raylib.h"
#include <stdio.h>

// Sabitleri burada tanımlıyoruz ki değiştirmesi kolay olsun
#define HUCRE_BOYUTU 30 //her bir hucre kare ve kenarı ,30
#define HUCRE_SAYISI 25 // X * X hucrelerden olusan bir ekran ,25
#define EKRAN_GENISLIK (HUCRE_BOYUTU * HUCRE_SAYISI)  //
#define EKRAN_YUKSEKLIK (HUCRE_BOYUTU * HUCRE_SAYISI) //
#define YILAN_MAX_UZUNLUGU HUCRE_SAYISI * HUCRE_SAYISI + 10 //+10 diyerek oyun cokmelerıne karsı bir onlem
#define YILAN_BASLANGIC_UZUNLUGU 2
#define MAX_YEM_SAYISI 5
#define BOYUT_ARTTIRICI_YEM_SANSI 20
#define HIZLANDIRICI_YEM_SANSI 15
#define HAREKETLI_YEM_SANSI 15

typedef enum { SAHNE_MENU, SAHNE_OYUN, SAHNE_BITIS, SAHNE_KAZANDIN } Sahne;

typedef struct {
    Vector2 konum; //cizerken isimize yarayacak
    Vector2 govde[YILAN_MAX_UZUNLUGU];
    Vector2 hiz;
    Color renk;
    int govde_uzunlugu;
} Yilan;


typedef struct {

    Vector2 konum;
    Vector2 hiz; //hareketli yem icin
    Color renk;
    bool aktif;
    int yem_turu;
    int YemKareZamanSayaci;
    int YemKareZamanSinirlayici;
} Yem;


typedef enum {
    standart_yem, //0
    boyut_arttirici_yem, //1
    hizlandirici_yem, //2
    hareketli_yem, //3

} YemTuru;




typedef struct {

    Yilan yilan;
    Yem yemler[MAX_YEM_SAYISI]; //en cok kullanılacak olanlardan. genel olarak tum yemlerı kapsar
    int skor;
    int MaxSkor;
    bool OyunDurumu;
    int ZamanKareSayaci; //Yilan cok hizli gitmemesi icin bir kontol sayaci
    int ZamanKareSinirlayicisi; //Oyun akışını yani yilanin hizini ayarlamak icin sınırlayıcı
    Sahne guncelSahne;

    //DOKULAR
    Texture2D dokuAcikCim;
    Texture2D dokuKoyuCim;
    Texture2D dokuKafa;
    Texture2D dokuGovde;
    Texture2D dokuElma;    // Standart yem
    Texture2D dokuMuz;     // Boyut arttırıcı yem
    Texture2D dokuKiraz;   // Hızlandırıcı yem
    Texture2D dokuPortakal;// Hareketli yem

    //SESLER
    Sound sesYeme;    // Yem yendiğinde
    Sound sesCarpma;  // Yılan kendine çarptığında
    Sound sesKazanma; // Oyun bittiğinde

    Music muzikArkaPlan; //Arka Plan muzigi

} Oyun; //Oyunun içinde rahat gezmek için hepsini bir arada topladık. (*oyun Pointerini kullanarak Oyun icerisinde gezecegiz)


//FONKSIYONLAR

void ArkaPlanCiz (Oyun *oyun) {
    for (int x = 0; x < HUCRE_SAYISI; x++) {
        for (int y = 0; y < HUCRE_SAYISI; y++) {
            // Hangi dokuyu kullanacağımızı seçiyoruz
            Texture2D aktifDoku;
            if ((x + y) % 2 == 0) {
                aktifDoku = oyun->dokuAcikCim;
            } else {
                aktifDoku = oyun->dokuKoyuCim;
            }

            // Nereye çizileceğini (Hücre konumu) belirliyoruz
            Rectangle hedef = { x * HUCRE_BOYUTU, y * HUCRE_BOYUTU, HUCRE_BOYUTU, HUCRE_BOYUTU };

            // Resmin tamamını kaynak olarak alıyoruz
            Rectangle kaynak = { 0, 0, (float)aktifDoku.width, (float)aktifDoku.height };

            DrawTexturePro(aktifDoku, kaynak, hedef, (Vector2){0, 0}, 0.0f, WHITE);
        }
    }
}



void HareketliYemiCalistir (Oyun *oyun) {

      for (int i = 0; i<MAX_YEM_SAYISI;i++) {
        if (oyun->yemler[i].aktif && oyun->yemler[i].yem_turu == hareketli_yem) { //hareketli yemi bulduk

            oyun->yemler[i].YemKareZamanSayaci++;
            if (oyun->yemler[i].YemKareZamanSayaci>=oyun->yemler[i].YemKareZamanSinirlayici) { //her belirli sayida hareket et
                oyun->yemler[i].YemKareZamanSayaci = 0;

                Vector2 yeni_konum = {oyun->yemler[i].konum.x + oyun->yemler[i].hiz.x,
                                        oyun->yemler[i].konum.y + oyun->yemler[i].hiz.y}; //yılanın yenı konumunu belirle.

                //duvarlardan cıkıyorsa diger taraftan gelsinler
                if (yeni_konum.x >= HUCRE_SAYISI) yeni_konum.x = 0;
                else if (yeni_konum.x < 0) yeni_konum.x = HUCRE_SAYISI-1;
                if (yeni_konum.y>= HUCRE_SAYISI) yeni_konum.y = 0;
                else if (yeni_konum.y < 0) yeni_konum.y = HUCRE_SAYISI-1;

                //yılanın ıcıne girmesinler
                bool yilana_carpti = false;
                for (int j = 0; j<oyun->yilan.govde_uzunlugu;j++) { //yılanın her parcasına bak
                    if (yeni_konum.x == oyun->yilan.govde[j].x && yeni_konum.y == oyun->yilan.govde[j].y ) {
                        yilana_carpti = true;
                        break; //yilana carptiysa donguden cık
                    }
                }

                //eger yılana carpmıyorsa
                if (!yilana_carpti) {
                    oyun->yemler[i].konum = yeni_konum;
                }
                else { //eger yılana carpıyorsa
                    int yon = GetRandomValue(0,3); //rastgele bir yon belirle
                    switch(yon) {
                        case 0: oyun->yemler[i].hiz = (Vector2){0, 1}; break;   // Sadece aşağı
                        case 1: oyun->yemler[i].hiz = (Vector2){0, -1}; break;  // Sadece yukarı
                        case 2: oyun->yemler[i].hiz = (Vector2){1, 0}; break;   // Sadece sağ
                        case 3: oyun->yemler[i].hiz = (Vector2){-1, 0}; break;  // Sadece sol
                    }
                }

                //hareketli yemin normal hareketi
                if (GetRandomValue(1,100) <= 10) { //%10 ihtimalle yon degistirecegiz
                    int yon = GetRandomValue(0,3);
                    switch(yon) {
                        case 0: oyun->yemler[i].hiz = (Vector2){0, 1}; break;   // Sadece aşağı
                        case 1: oyun->yemler[i].hiz = (Vector2){0, -1}; break;  // Sadece yukarı
                        case 2: oyun->yemler[i].hiz = (Vector2){1, 0}; break;   // Sadece sağ
                        case 3: oyun->yemler[i].hiz = (Vector2){-1, 0}; break;  // Sadece sol
                    }
                }
        }
      }
    }
}


void YemOlustur(Oyun *oyun, int indeks) {

    bool gecerli_konum = false;
    int rastgeleX;
    int rastgeleY;

    while (!gecerli_konum) { //gecerli konum dogruyken calıs
        gecerli_konum = true;
        rastgeleX = GetRandomValue(0, HUCRE_SAYISI - 1);
        rastgeleY = GetRandomValue(0, HUCRE_SAYISI - 1);

        for (int i = 1;i<oyun->yilan.govde_uzunlugu;i++) {  //YemleriDoldur fonksiyonu ile indeks degisikligi yapıyoruz.
            if(oyun->yilan.govde[i].x == rastgeleX && oyun->yilan.govde[i].y == rastgeleY ){
                gecerli_konum = false; //yem yilanin icinde olmaması icin kontrol
                break; // donguyu kır ve tekrar yem ıcın uygun konum ara
            }
        }

        oyun->yemler[indeks].konum = (Vector2) {rastgeleX,rastgeleY};
        oyun->yemler[indeks].aktif = true;
        int ozellik_sansi = GetRandomValue(1,100);

        if (ozellik_sansi <= BOYUT_ARTTIRICI_YEM_SANSI) { //
            oyun->yemler[indeks].yem_turu = boyut_arttirici_yem;
            oyun->yemler[indeks].renk = GREEN;
        }

        else if (ozellik_sansi <= HIZLANDIRICI_YEM_SANSI + BOYUT_ARTTIRICI_YEM_SANSI) { //ihtimaller toplanıyor ve else if ile kıstırıyoruz
            oyun->yemler[indeks].yem_turu = hizlandirici_yem;
            oyun->yemler[indeks].renk = YELLOW;
        }
        else if (ozellik_sansi<= HAREKETLI_YEM_SANSI + HIZLANDIRICI_YEM_SANSI + BOYUT_ARTTIRICI_YEM_SANSI) { //
            oyun->yemler[indeks].yem_turu = hareketli_yem;
            oyun->yemler[indeks].renk = PINK;
            oyun->yemler[indeks].hiz = (Vector2) {1,0}; //ilk hız vererek baslat
            oyun->yemler[indeks].YemKareZamanSayaci = 0;
            oyun->yemler[indeks].YemKareZamanSinirlayici = 15;
        }

        else {
            oyun->yemler[indeks].yem_turu = standart_yem;
            oyun->yemler[indeks].renk = BLUE;
        }

    } //while sonu


}


void YemleriOlustur(Oyun *oyun) { //hem sifirlama hem doldurma islemi.

    for (int i =0;i< MAX_YEM_SAYISI;i++) {
        YemOlustur(oyun,i);
    }

}


void YemleriCizdir(Oyun *oyun) {

    for (int i = 0; i<MAX_YEM_SAYISI;i++) {
        if (oyun->yemler[i].aktif) {
            Texture2D aktifDoku; //aktif dokuyu yemin cesidine gore atayacagız

            switch (oyun->yemler[i].yem_turu) {
            case boyut_arttirici_yem:
                aktifDoku = oyun->dokuMuz;
                break;
            case hizlandirici_yem:
                aktifDoku = oyun->dokuKiraz;
                break;
            case hareketli_yem:
                aktifDoku = oyun->dokuPortakal;
                break;
            default:
                aktifDoku = oyun->dokuElma;
                break;
            }

            float meyveBoyutu = HUCRE_BOYUTU * 1.4f;
            float dengeleme = (HUCRE_BOYUTU - meyveBoyutu) / 2.0f; // Boşluğu ikiye böl ki merkeze gelsin

            Rectangle hedef = {
                oyun->yemler[i].konum.x * HUCRE_BOYUTU + dengeleme,
                oyun->yemler[i].konum.y * HUCRE_BOYUTU + dengeleme,
                meyveBoyutu,
                meyveBoyutu
            };

            Rectangle kaynak = { 0, 0, (float)aktifDoku.width, (float)aktifDoku.height };

            DrawTexturePro(aktifDoku, kaynak, hedef, (Vector2){0, 0}, 0.0f, WHITE);
        }
    }
}


void YilanSifirla (Oyun *oyun) {

    oyun->yilan.govde_uzunlugu = YILAN_BASLANGIC_UZUNLUGU;
    oyun->yilan.renk = RED;
    oyun->yilan.hiz = (Vector2){1,0}; //baslangıcta hareket ettirme,sabit baslamasın.
    oyun->yilan.govde[0] = (Vector2) {HUCRE_SAYISI/2,HUCRE_SAYISI/2}; //Kafa baslangic hucresi
    //Yılan gövdeleri direkt bir konumu değil bir hücreyi ifade etmekte
}


void OyunSifirla (Oyun *oyun) {

    YilanSifirla(oyun); //icine bir adres gondermeliyiz. burada yazan oyun bir pointer yani adres.
    YemleriOlustur(oyun); //oyun sıfırlanınca yemleri doldurur
    oyun->OyunDurumu = true;
    oyun->skor = 0;
    oyun->ZamanKareSayaci = 0;
    oyun->ZamanKareSinirlayicisi=7;

}


void YilanGuncelle(Oyun *oyun) {

    if (IsKeyPressed(KEY_UP) && oyun->yilan.hiz.y == 0) { //y ekseninde hareket yoksa y ekseninde hareket et. aksi halde, yilan kendine girmeye calisir
        oyun->yilan.hiz = (Vector2) {0,-1};
    }
    if (IsKeyPressed(KEY_DOWN) && oyun->yilan.hiz.y == 0) {
        oyun->yilan.hiz = (Vector2) {0,1};
    }
    if (IsKeyPressed(KEY_LEFT) && oyun->yilan.hiz.x == 0) { //x ekseninde hareket yoksa x ekseninde hareket et
        oyun->yilan.hiz = (Vector2) {-1,0};
    }
    if (IsKeyPressed(KEY_RIGHT) && oyun->yilan.hiz.x == 0) {
        oyun->yilan.hiz = (Vector2) {1,0};
    }

   oyun->ZamanKareSayaci++;
    if (oyun->ZamanKareSayaci >= oyun->ZamanKareSinirlayicisi) { //her belirlenen karede bir hareket et
        oyun->ZamanKareSayaci = 0; //sifirlama islemi

        for (int i = oyun->yilan.govde_uzunlugu - 1;i>0;i--) { //Govde kaydirma
        oyun->yilan.govde[i] = oyun->yilan.govde[i-1]; // Kuyruktan kafaya dogru
        }

        oyun->yilan.govde[0].x += oyun->yilan.hiz.x; // Kafayi x ekseninde hareket ettirme
        oyun->yilan.govde[0].y += oyun->yilan.hiz.y; // Kafayi y ekseninde hareket ettirme

    }

}


void YilanCizdir(Oyun *oyun) {
    for (int i = 0; i < oyun->yilan.govde_uzunlugu; i++) {
        // hücreleri koordinata cevirme islemi
        float baseX = oyun->yilan.govde[i].x * HUCRE_BOYUTU;
        float baseY = oyun->yilan.govde[i].y * HUCRE_BOYUTU;

        //Kafa cizimi
        if (i == 0) { //govde[0] kafaydi

            Rectangle hedefKafa = { baseX, baseY, HUCRE_BOYUTU, HUCRE_BOYUTU };
            Rectangle kaynakKafa = { 0, 0, (float)oyun->dokuKafa.width, (float)oyun->dokuKafa.height };

            DrawTexturePro(oyun->dokuKafa, kaynakKafa, hedefKafa, (Vector2){0,0}, 0.0f, WHITE);

        } else { //govde cizimi

            float buyutucu = 1.1f; //her bir govde parcasının ust uste bınmesı ıcın buyuttuk
            float yeniBoyut = HUCRE_BOYUTU * buyutucu;
            float dengeleme = (HUCRE_BOYUTU - yeniBoyut) / 2.0f; // Merkeze hizalamak için

            Rectangle hedefGovde = {
                baseX + dengeleme,
                baseY + dengeleme,
                yeniBoyut,
                yeniBoyut
            };
            Rectangle kaynakGovde = { 0, 0, (float)oyun->dokuGovde.width, (float)oyun->dokuGovde.height };

            // Piksellenmeyi önlemek için filtre
            // SetTextureFilter(oyun->dokuGovde, TEXTURE_FILTER_BILINEAR);
            DrawTexturePro(oyun->dokuGovde, kaynakGovde, hedefGovde, (Vector2){0,0}, 0.0f, WHITE);
        }
    }
}


void RekoruYukle(Oyun *oyun) {

    FILE *dosya = fopen("rekor.txt", "r"); //r okuma modu

    if (dosya != NULL) { //dosya varsa
        fscanf(dosya, "%d", &oyun->MaxSkor);
        fclose(dosya);
    } else { //dosya yoksa basta 0 ata.
        oyun->MaxSkor = 0;
    }
}


void RekoruKaydet(Oyun *oyun) {

    FILE *dosya = fopen("rekor.txt", "w"); //w yazma modu uzerıne yazar

    if (dosya != NULL) {
        fprintf(dosya, "%d", oyun->MaxSkor);
        fclose(dosya);
    }
}



bool CarpismaAlgila (Oyun *oyun) { //Carpinca oyunun bittigini kontrol ettigimiz fonksiyon

    //yilan.govde[0] yılanın kafası demek oluyor

    if (oyun->yilan.govde[0].x >= HUCRE_SAYISI) { //x ekseninde sagdan taşma
        oyun->yilan.govde[0].x = 0;
    }

    if (oyun->yilan.govde[0].x < 0) {     //x ekseninde soldan taşma
        oyun->yilan.govde[0].x = HUCRE_SAYISI-1;
    }

    if (oyun->yilan.govde[0].y >= HUCRE_SAYISI) { //y ekseninde alttan taşma
        oyun->yilan.govde[0].y = 0;
    }

    if (oyun->yilan.govde[0].y < 0) { //y ekseninde yukarıdan taşma
        oyun->yilan.govde[0].y = HUCRE_SAYISI-1;
    }

    for (int i = 1; i<oyun->yilan.govde_uzunlugu;i++) {
        if (oyun->yilan.govde[i].x == oyun->yilan.govde[0].x  && oyun->yilan.govde[i].y == oyun->yilan.govde[0].y) { //kendine carpma kontrolu

            PlaySound(oyun->sesCarpma);

            if (oyun->skor > oyun->MaxSkor) { //rekor kırma kontrolu
                oyun->MaxSkor = oyun->skor;
                RekoruKaydet(oyun);
            }

            return true; //carpısma olduysa true
        }
    }

    return false; //carpısma yoksa false
}


void YemYemeKontrol (Oyun *oyun){

    for (int i = 0;i<MAX_YEM_SAYISI;i++) {
        if (oyun->yemler[i].aktif) {
            if (oyun->yilan.govde[0].x == oyun->yemler[i].konum.x  &&  oyun->yilan.govde[0].y == oyun->yemler[i].konum.y) { //yem yendiyse

                PlaySound(oyun->sesYeme);

                int buyume_miktari = 1;
                if (oyun->yemler[i].yem_turu == boyut_arttirici_yem) {
                    oyun->skor +=3;
                    buyume_miktari = 3;
                    oyun->ZamanKareSinirlayicisi = 7;
                }

                else if (oyun->yemler[i].yem_turu == hizlandirici_yem) {
                    oyun->skor +=1;
                    buyume_miktari=1;
                    oyun->ZamanKareSinirlayicisi = 5;
                }

                else if (oyun->yemler[i].yem_turu == hareketli_yem) {
                    oyun->skor +=1;
                    buyume_miktari=1;
                    oyun->ZamanKareSinirlayicisi = 7;
                }

                else { //standart yem yerse
                    oyun->skor +=1;
                    buyume_miktari =1;
                    oyun->ZamanKareSinirlayicisi = 7;
                }
                for (int j=0;j<buyume_miktari;j++) {
                    int MevcutSonKuyruk = oyun->yilan.govde_uzunlugu - 1;
                    oyun->yilan.govde[oyun->yilan.govde_uzunlugu] = oyun->yilan.govde[MevcutSonKuyruk]; //yilanin eklenecegi yer
                    oyun->yilan.govde_uzunlugu++; //eklenecek yer belirlendikten sonra uzunluk arttirildi.

                }

                oyun->yemler[i].aktif = false; //o yem yendi ve kapandi
                YemOlustur(oyun,i); //yenisi olusturuldu
            }
        }
    }

    if (oyun->yilan.govde_uzunlugu >= HUCRE_SAYISI * HUCRE_SAYISI) { //yılan ekranın tamamını kaplıyorsa oyunu kazandın
        PlaySound(oyun->sesKazanma);
        oyun->guncelSahne = SAHNE_KAZANDIN;
    }
}


void ResimleriYukle(Oyun *oyun) {

    Image resimAcikCim = GenImageColor(HUCRE_BOYUTU,HUCRE_BOYUTU,(Color){170, 215, 81, 255});
    oyun->dokuAcikCim = LoadTextureFromImage(resimAcikCim);
    UnloadImage(resimAcikCim); //artık image ihtiyacımız yok,dokuya atamıs olduk zaten


    Image resimKoyuCim = GenImageColor(HUCRE_BOYUTU,HUCRE_BOYUTU,(Color){162, 209, 73, 255});
    oyun->dokuKoyuCim = LoadTextureFromImage(resimKoyuCim);
    UnloadImage(resimKoyuCim);

    //yılanın dokusunu dısarıdan almayıp raylib ile olusturduk
    //yılan govde olusumu
    Image imgGovde = GenImageGradientRadial(HUCRE_BOYUTU, HUCRE_BOYUTU, 0.0f, (Color){0, 200, 0, 255}, (Color){0, 100, 0, 255});
    oyun->dokuGovde = LoadTextureFromImage(imgGovde);
    UnloadImage(imgGovde); //image e gerek kalmadı dokuya atadık zaten

    //yılan kafa olusumu
    Image imgKafa = GenImageColor(HUCRE_BOYUTU, HUCRE_BOYUTU, (Color){0, 220, 50, 255});
    //yılan gozleri olusumu
    int gozBoyutu = HUCRE_BOYUTU / 4;
    int gozBebegiBoyutu = gozBoyutu / 2;
    int solGozX = HUCRE_BOYUTU / 4 - gozBoyutu/2;
    int sagGozX = (HUCRE_BOYUTU * 3) / 4 - gozBoyutu/2;
    int gozY = HUCRE_BOYUTU / 4;

    // sol goz beyazı
    ImageDrawRectangle(&imgKafa, solGozX, gozY, gozBoyutu, gozBoyutu, WHITE);
    // sag goz beyazı
    ImageDrawRectangle(&imgKafa, sagGozX, gozY, gozBoyutu, gozBoyutu, WHITE);

    // sol goz siyahı
    ImageDrawRectangle(&imgKafa, solGozX + gozBebegiBoyutu/2, gozY + gozBebegiBoyutu/2, gozBebegiBoyutu, gozBebegiBoyutu, BLACK);
    // sag goz siyahı
    ImageDrawRectangle(&imgKafa, sagGozX + gozBebegiBoyutu/2, gozY + gozBebegiBoyutu/2, gozBebegiBoyutu, gozBebegiBoyutu, BLACK);

    // Oluşturduğumuz bu gözlü resmi dokuya çevirme islemi
    oyun->dokuKafa = LoadTextureFromImage(imgKafa);
    UnloadImage(imgKafa); // resmi olusturduk,dokuya atadık , artık resmi temizledik

    //meyveleri dosyadan alma islemi
    oyun->dokuElma = LoadTexture("resimler/elma.png");
    oyun->dokuMuz = LoadTexture("resimler/muz.png");
    oyun->dokuKiraz = LoadTexture("resimler/kiraz.png");
    oyun->dokuPortakal = LoadTexture("resimler/portakal.png");


}


void ResimleriTemizle(Oyun *oyun) {

    UnloadTexture(oyun->dokuAcikCim);
    UnloadTexture(oyun->dokuKoyuCim);
    UnloadTexture(oyun->dokuKafa);
    UnloadTexture(oyun->dokuGovde);
    UnloadTexture(oyun->dokuElma);
    UnloadTexture(oyun->dokuMuz);
    UnloadTexture(oyun->dokuKiraz);
    UnloadTexture(oyun->dokuPortakal);

}


void SesleriYukle(Oyun *oyun) {

    InitAudioDevice(); //ses cihazını baslat

    oyun->sesYeme = LoadSound("sesler/yeme.wav");
    oyun->sesCarpma = LoadSound("sesler/carpma.wav");
    oyun->sesKazanma = LoadSound("sesler/kazanma.wav");

    oyun->muzikArkaPlan = LoadMusicStream("sesler/arkaplan.mp3");
    oyun->muzikArkaPlan.looping = true; //muzık bıtınce tekrar baslaması ıcın

}

void SesleriTemizle(Oyun *oyun) {
    UnloadSound(oyun->sesYeme);
    UnloadSound(oyun->sesCarpma);
    UnloadSound(oyun->sesKazanma);
    UnloadMusicStream(oyun->muzikArkaPlan);

    CloseAudioDevice(); //ses cihazını kapat
}




void SkorCizdir (Oyun *oyun) {
    char mesaj[50];
    sprintf(mesaj, "Skor: %d  En Yuksek: %d", oyun->skor, oyun->MaxSkor);
    DrawText(mesaj, 10, 10, 20, DARKGRAY);
}


int main(void) {
    Oyun oyun;
    InitWindow(EKRAN_GENISLIK, EKRAN_YUKSEKLIK, "YILAN OYUNU");
    SetTargetFPS(60);

    ResimleriYukle(&oyun);
    SesleriYukle(&oyun);
    RekoruYukle(&oyun);

    oyun.guncelSahne = SAHNE_MENU; // Oyun menü ile başlasın

    while (!WindowShouldClose()) {
        // OYUN SAHNELERI KONTROL
        switch (oyun.guncelSahne) {
            case SAHNE_MENU:
                StopMusicStream(oyun.muzikArkaPlan);
                if (IsKeyPressed(KEY_ENTER)) {
                    OyunSifirla(&oyun);
                    oyun.guncelSahne = SAHNE_OYUN;
                    PlayMusicStream(oyun.muzikArkaPlan);
                }
                break;

            case SAHNE_OYUN:
                UpdateMusicStream(oyun.muzikArkaPlan);
                YilanGuncelle(&oyun);
                if (CarpismaAlgila(&oyun)) {
                    oyun.guncelSahne = SAHNE_BITIS;
                }
                HareketliYemiCalistir(&oyun);
                YemYemeKontrol(&oyun);
                break;

            case SAHNE_BITIS:
            case SAHNE_KAZANDIN:
                StopMusicStream(oyun.muzikArkaPlan); //bitince ya da kazanınca muzık dursun
                if (IsKeyPressed(KEY_R)) {
                    oyun.guncelSahne = SAHNE_MENU;
                }
                break;
        }

        // CIZIM
        BeginDrawing();
        ClearBackground(RAYWHITE);

        switch (oyun.guncelSahne) {
            case SAHNE_MENU:
                ArkaPlanCiz(&oyun); // Menü arkasında çimler gözüksün
                DrawRectangle(0, 0, EKRAN_GENISLIK, EKRAN_YUKSEKLIK, Fade(BLACK, 0.6f));
                DrawText("YILAN OYUNU", EKRAN_GENISLIK/2 - 150, 200, 50, GREEN);
                DrawText("BASLAMAK ICIN 'ENTER' BAS", EKRAN_GENISLIK/2 - 180, 350, 25, WHITE);
                DrawText(TextFormat("EN YUKSEK SKOR: %d", oyun.MaxSkor), EKRAN_GENISLIK/2 - 100, 450, 20, YELLOW);
                break;

            case SAHNE_OYUN:
                ArkaPlanCiz(&oyun);
                YilanCizdir(&oyun);
                YemleriCizdir(&oyun);
                SkorCizdir(&oyun);
                break;

            case SAHNE_BITIS:
                ArkaPlanCiz(&oyun);
                DrawRectangle(0, 0, EKRAN_GENISLIK, EKRAN_YUKSEKLIK, Fade(BLACK, 0.7f));
                DrawText("EYVAH! CARPTIN", EKRAN_GENISLIK/2 - 180, 250, 45, RED);
                DrawText(TextFormat("SKORUN: %d", oyun.skor), EKRAN_GENISLIK/2 - 60, 320, 25, WHITE);
                DrawText("MENUYE DONMEK ICIN 'R' BAS", EKRAN_GENISLIK/2 - 170, 450, 20, LIGHTGRAY);
                break;

            case SAHNE_KAZANDIN:
                ArkaPlanCiz(&oyun);
                DrawRectangle(0, 0, EKRAN_GENISLIK, EKRAN_YUKSEKLIK, Fade(GOLD, 0.5f));
                DrawText("TEBRIKLER! KAZANDIN!", EKRAN_GENISLIK/2 - 250, 250, 45, WHITE);
                DrawText("EFSANEVI BIR OYUNCUSUN!", EKRAN_GENISLIK/2 - 180, 320, 25, DARKGREEN);
                DrawText("MENUYE DONMEK ICIN 'R' BAS", EKRAN_GENISLIK/2 - 170, 450, 20, DARKGRAY);
                break;
        }

        EndDrawing();
    }

    ResimleriTemizle(&oyun);
    SesleriTemizle(&oyun);
    CloseWindow();

    return 0;
}
