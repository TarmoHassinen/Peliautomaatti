//
//  peliautomaatti.cpp
//
//  Created by Tarmo Hassinen on 14/03/2019.
//  Copyright © 2019 TarmoH. All rights reserved.
//
//SDL ja standard IO
#include <SDL2/SDL.h>
#include <SDL2/SDL_log.h>
#include <SDL2_image/SDL_image.h>
#include <SDL2_ttf/SDL_ttf.h>
#include <SDL2_mixer/SDL_mixer.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <stdlib.h>
#include <time.h>
#include <cmath>
#include <sstream>
#include <iomanip>

using namespace std;

//Ruudun mitat vakiot
const int SCREEN_WIDTH = 850;
const int SCREEN_HEIGHT = 650;


/*Tämä koodinpätkä (class LTexture) alun perin Lazy Foo' Productionsilta
 (http://lazyfoo.net/)*/

//Tekstuuri luokka
class LTexture
{
public:
    //Muuttujien alustus
    LTexture();
    
    //Vapauttaa muistin
    ~LTexture();
    
    //Lataa kuvan annetusta polusta
    bool loadFromFile( std::string path );
    
    //Luo kuvan fontti merkkijonosta
    bool loadFromRenderedText( std::string textureText, SDL_Color textColor );
    
    //Vapauttaa tekstuurin
    void free();
    
    //Värien moduloinnin asettaminen
    void setColor( Uint8 red, Uint8 green, Uint8 blue );
    
    //Blendauksen asettaminen
    void setBlendMode( SDL_BlendMode blending );
    
    //Alpha moduloinnin asettaminen
    void setAlpha( Uint8 alpha );
    
    //Renderöi tekstuurin annetussa pisteessä
    void render( int x, int y, SDL_Rect* clip = NULL, double angle = 0.0, SDL_Point* center = NULL, SDL_RendererFlip flip = SDL_FLIP_NONE );
    
    //Hakee kuvan mitat
    int getWidth();
    int getHeight();
    
private:
    //Hardware tekstuuri
    SDL_Texture* mTexture;
    
    //Kuvan mitat
    int mWidth;
    int mHeight;
};

//Käynnistää SDL:län ja luo ikkunan
bool init();

//Lataa median
bool loadMedia(); 

//Vapauttaa median ja sulkee SDL:län
void close();

//Lataa tietyn kuvan tekstuurina
SDL_Texture* loadTexture( std::string path );

//Ikkuna, johon renderöidään
SDL_Window* gWindow = NULL;

//Ikkunan renderöijä
SDL_Renderer* gRenderer = NULL;

//Globaalisti käytettävä fontti
TTF_Font *gFont = NULL;

//Käytettävät ääniefektit
Mix_Chunk *RullaPyorii = NULL;
Mix_Chunk *RullaPysahtyyChunk = NULL;
Mix_Chunk *NapinPainallus = NULL;
Mix_Chunk *PieniVoitto = NULL;

//Alustus muuttujille, jotka kertovat, mihin kuvioon rullat ovat osuneet
int ApilaOsumat;
int TahtiOsumat;
int MansikkaOsumat;
int MeloniOsumat;
int BanaaniOsumat;
int ViinirypaleOsumat;

//Alustus muuttujille, joilla lasketaan yhteen kuvioiden osumat ja voittokerroin
double ApilaVoittoKerroin;
double TahtiVoittoKerroin;
double MansikkaVoittoKerroin;
double MeloniVoittoKerroin;
double BanaaniVoittoKerroin;
double ViinirypaleVoittoKerroin;

//Voiton määrän, panoksen määrän ja saldon määrän alustus
double VoittoMaara;
double PanosMaara = 0.20;
double SaldoMaara = 20.00;

//Tekstien tekstuureiden renderöinti
LTexture voitto;
LTexture panos;
LTexture saldo;

//Taustakuvan tekstuurin renderöinti
LTexture background;

//Eri kuviollisten rullien tekstuureiden renderöinti
LTexture ApilaKuvioRulla;
LTexture TahtiKuvioRulla;
LTexture MansikkaKuvioRulla;
LTexture MeloniKuvioRulla;
LTexture BanaaniKuvioRulla;
LTexture ViinirypaleKuvioRulla;

//Pyörivän rullan tekstuurin renderöinti
LTexture PyorivaRulla;

//Voittavan kuvion tekstuurin renderöinti
LTexture voittava_kuvio;
LTexture voittava_kuvio2;
LTexture voittava_kuvio3;

//Nappien tekstuurin renderöinti
LTexture PelaaNappi;
LTexture PanosNappi;
LTexture PelaaNappiPainettu;
LTexture PanosNappiPainettu;

LTexture::LTexture()
{
    //Alustus
    mTexture = NULL;
    mWidth = 0;
    mHeight = 0;
}

LTexture::~LTexture()
{
    //Vapautus
    free();
}


/*Tämä koodinpätkä (bool LTexture::loadFromFile) alun perin Lazy Foo' Productionsilta
 (http://lazyfoo.net/)*/

bool LTexture::loadFromFile( std::string path )
{
    //Poistaa entisen tekstuurin
    free();
    
    //Lopullinen tekstuuri
    SDL_Texture* newTexture = NULL;
    
    //Lataa kuvan annetusta polusta
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    else
    {
        //Kuvan läpinäkyvyyden asettaminen värin mukaan
        SDL_SetColorKey( loadedSurface, SDL_TRUE, SDL_MapRGB( loadedSurface->format, 255, 255, 255 ) );
        
        //Luo tekstuurin pinnan pikseleistä
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }
        else
        {
            //Hakee kuvan mitat
            mWidth = loadedSurface->w;
            mHeight = loadedSurface->h;
        }
        
        //Poistaa vanhan ladatun pinnan
        SDL_FreeSurface( loadedSurface );
    }
    
    //Palauttaa tiedon onnistumisesta
    mTexture = newTexture;
    return mTexture != NULL;
}

bool LTexture::loadFromRenderedText( std::string textureText, SDL_Color textColor )
{
    //Poistaa entisen tekstuurin
    free();
    
    //Renderöi tekstin pinnan
    SDL_Surface* textSurface = TTF_RenderText_Solid( gFont, textureText.c_str(), textColor );
    if( textSurface == NULL )
    {
        printf( "Unable to render text surface! SDL_ttf Error: %s\n", TTF_GetError() );
    }
    else
    {
        //Luo tekstuurin pinnan pikseleistä
        mTexture = SDL_CreateTextureFromSurface( gRenderer, textSurface );
        if( mTexture == NULL )
        {
            printf( "Unable to create texture from rendered text! SDL Error: %s\n", SDL_GetError() );
        }
        else
        {
            //Hakee kuvan mitat
            mWidth = textSurface->w;
            mHeight = textSurface->h;
        }
        
        //Poistaa vanhan pinnan
        SDL_FreeSurface( textSurface );
    }
    
    //Palauttaa tiedon onnistumisesta
    return mTexture != NULL;
}

void LTexture::free()
{
    //Vapauttaa tekstuurin, jos se on olemassa
    if( mTexture != NULL )
    {
        SDL_DestroyTexture( mTexture );
        mTexture = NULL;
        mWidth = 0;
        mHeight = 0;
    }
}

void LTexture::setColor( Uint8 red, Uint8 green, Uint8 blue )
{
    //Moduloi tekstuurin RGB:tä
    SDL_SetTextureColorMod( mTexture, red, green, blue );
}

void LTexture::setBlendMode( SDL_BlendMode blending )
{
    //Asettaa blendauksen funktion
    SDL_SetTextureBlendMode( mTexture, blending );
}

void LTexture::setAlpha( Uint8 alpha )
{
    //Moduloi tekstuurin läpinäkyvyyttä
    SDL_SetTextureAlphaMod( mTexture, alpha );
}

void LTexture::render( int x, int y, SDL_Rect* clip, double angle, SDL_Point* center, SDL_RendererFlip flip )
{
    //Asettaa renderöintialueen ja renderöi näytölle
    SDL_Rect renderQuad = { x, y, mWidth, mHeight };
    
    //Asettaa klipin renderöinnin mitat
    if( clip != NULL )
    {
        renderQuad.w = clip->w;
        renderQuad.h = clip->h;
    }
    
    //Renderöi näytölle
    SDL_RenderCopyEx( gRenderer, mTexture, clip, &renderQuad, angle, center, flip );
}

int LTexture::getWidth()
{
    return mWidth;
}

int LTexture::getHeight()
{
    return mHeight;
}


/*Tämä koodinpätkä (bool init()) alun perin Lazy Foo' Productionsilta
 (http://lazyfoo.net/)*/

bool init()
{
    //Ilmoitus alustuksesta
    bool success = true;
    
    //SDL:län alustus
    if( SDL_Init( SDL_INIT_VIDEO | SDL_INIT_AUDIO ) < 0 )
    {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
        success = false;
    }
    else
    {

        //Asettaa tekstuurin suodatuksen lineaariseksi
        if( !SDL_SetHint( SDL_HINT_RENDER_SCALE_QUALITY, "1" ) )
        {
            printf( "Warning: Linear texture filtering not enabled!" );
        }
        //Luo ikkunan
        gWindow = SDL_CreateWindow( "Peliautomaatti", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN );
        if( gWindow == NULL )
        {
            printf( "Window could not be created! SDL_Error: %s\n", SDL_GetError() );
            success = false;
        }
        else
        {
            
            //Luo ikkunan renderöijä
            gRenderer = SDL_CreateRenderer( gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC );
            if( gRenderer == NULL )
            {
                printf( "Renderer could not be created! SDL Error: %s\n", SDL_GetError() );
                success = false;
            }
            else
            {
                //Alusta renderöijän väri
                SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
                
                //Alusta PNG-lataus
                int imgFlags = IMG_INIT_PNG;
                if( !( IMG_Init( imgFlags ) & imgFlags ) )
                {
                    printf( "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError() );
                    success = false;
                }
                
                //Alusta SDL_ttf
                if( TTF_Init() == -1 )
                {
                    printf( "SDL_ttf could not initialize! SDL_ttf Error: %s\n", TTF_GetError() );
                    success = false;
                }
                
                //Alusta SDL_mixer
                if( Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ) < 0 )
                {
                    printf( "SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError() );
                    success = false;
                }
            }
        }
    }
    
    return success;
}

bool loadMedia()
{
    //Lataamisen onnistumisen ilmoitus
    bool success = true;
    
    //Kuvien lataaminen polusta
    background.loadFromFile( "Media/background.png" );
    ApilaKuvioRulla.loadFromFile( "Media/apilakuvio.png" );
    PelaaNappi.loadFromFile( "Media/pelaa_nappi.png" );
    PanosNappi.loadFromFile( "Media/panos_nappi.png" );
    PelaaNappiPainettu.loadFromFile( "Media/pelaa_nappi_painettu.png" );
    PanosNappiPainettu.loadFromFile( "Media/panos_nappi_painettu.png" );
    TahtiKuvioRulla.loadFromFile( "Media/tahtikuvio.png" );
    MansikkaKuvioRulla.loadFromFile( "Media/mansikkakuvio.png" );
    MeloniKuvioRulla.loadFromFile( "Media/melonikuvio.png" );
    BanaaniKuvioRulla.loadFromFile( "Media/banaanikuvio.png" );
    ViinirypaleKuvioRulla.loadFromFile( "Media/viinirypalekuvio.png" );
    PyorivaRulla.loadFromFile( "Media/pyoriva_rulla.png" );
    
    //Ääniefektien lataaminen poluista
    RullaPyorii = Mix_LoadWAV( "Media/rulla_pyorii.wav" );
    if( RullaPyorii == NULL )
    {
        printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }
    
    RullaPysahtyyChunk = Mix_LoadWAV( "Media/rulla_pysahtyy.wav" );
    if( RullaPysahtyyChunk == NULL )
    {
        printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }
    
    PieniVoitto = Mix_LoadWAV( "Media/pieni_voitto.wav" );
    if( PieniVoitto == NULL )
    {
        printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }
    
    NapinPainallus = Mix_LoadWAV( "Media/napin_painallus.wav" );
    if( NapinPainallus == NULL )
    {
        printf( "Failed to load scratch sound effect! SDL_mixer Error: %s\n", Mix_GetError() );
        success = false;
    }
    
    //Avaa fontti
    gFont = TTF_OpenFont( "Media/LCD.ttf", 28 );
    if( gFont == NULL )
    {
        printf( "Failed to load lazy font! SDL_ttf Error: %s\n", TTF_GetError() );
        success = false;
    }
    else
    
    {
        //Renderöi voiton määrä -tekstin
        SDL_Color textColor = { 255, 0, 0 };
        if( !voitto.loadFromRenderedText( "-", textColor ) )
        {
            printf( "Failed to render text texture!\n" );
            success = false;
        }
    }
    
    {
        //Renderöi panoksen määrä -tekstin
        std::string out_string2;
        std::stringstream ss2;
        ss2 << PanosMaara;
        out_string2 = ss2.str();
        
        SDL_Color textColor = { 255, 0, 0 };
        
        if( !panos.loadFromRenderedText( ss2.str(), textColor ) )
        {
            printf( "Failed to render text texture!\n" );
            success = false;
        }
    }
    
    {
        //Renderöi saldon määrä -tekstin
        std::string out_string3;
        std::stringstream ss3;
        ss3 << SaldoMaara;
        out_string3 = ss3.str();
        
        SDL_Color textColor = { 255, 0, 0 };
        if( !saldo.loadFromRenderedText( ss3.str(), textColor ) )
        {
            printf( "Failed to render text texture!\n" );
            success = false;
        }
    }

    return success;
}

void close()
{
    //Vapauttaa ladatut kuvat teksteistä
    voitto.free();
    panos.free();
    saldo.free();
    
    //Vapauttaa ääniefektit
    Mix_FreeChunk( RullaPyorii );
    Mix_FreeChunk( RullaPysahtyyChunk );
    Mix_FreeChunk( NapinPainallus );
    Mix_FreeChunk( PieniVoitto );
    
    //Vapauttaa ladatut kuvat
    background.free();
    
    ApilaKuvioRulla.free();
    PelaaNappi.free();
    PanosNappi.free();
    PelaaNappiPainettu.free();
    PanosNappiPainettu.free();
    TahtiKuvioRulla.free();
    MansikkaKuvioRulla.free();
    MeloniKuvioRulla.free();
    BanaaniKuvioRulla.free();
    ViinirypaleKuvioRulla.free();
    PyorivaRulla.free();

    //Vapauttaa globaalin fontin
    TTF_CloseFont( gFont );
    gFont = NULL;
    
    //Tuhoaa ikkunan
    SDL_DestroyRenderer( gRenderer );
    SDL_DestroyWindow( gWindow );
    gWindow = NULL;
    gRenderer = NULL;

    //Lopettaa SDL:n alajärjestelmät
    Mix_Quit();
    TTF_Quit();
    IMG_Quit();
    SDL_Quit();
}

SDL_Texture* loadTexture( std::string path )
{
    //Lopullinen tekstuuri
    SDL_Texture* newTexture = NULL;
    
    //Lataa kuvan annetusta polusta
    SDL_Surface* loadedSurface = IMG_Load( path.c_str() );
    if( loadedSurface == NULL )
    {
        printf( "Unable to load image %s! SDL_image Error: %s\n", path.c_str(), IMG_GetError() );
    }
    else
    {
        //Luo tekstuuri pinnan pikseleistä
        newTexture = SDL_CreateTextureFromSurface( gRenderer, loadedSurface );
        if( newTexture == NULL )
        {
            printf( "Unable to create texture from %s! SDL Error: %s\n", path.c_str(), SDL_GetError() );
        }
        
        //Poista vanha ladattu pinta
        SDL_FreeSurface( loadedSurface );
    }
    
    return newTexture;
}

int main( int argc, char* args[] )
{
    //Käynnistä SDL ja luo ikkuna
    if( !init() )
    {
        printf( "Failed to initialize!\n" );
    }
    else
    {
        //Lataa media
        if( !loadMedia() )
        {
            printf( "Failed to load media!\n" );
        }
        else
        {
        
            bool quit = false;
            
            //SDL tapahtuman muuttujan esittely
            SDL_Event e;
            
            //Ääniefektien äänenvoimakkuuden asettaminen
            Mix_VolumeChunk(RullaPyorii, 20);
            Mix_VolumeChunk(RullaPysahtyyChunk, 7);
            Mix_VolumeChunk(NapinPainallus, 10);
            Mix_VolumeChunk(PieniVoitto, 30);
            Mix_VolumeMusic(7);
    
            while (!quit)
            {
                while (SDL_PollEvent(&e))
                {
                    if (e.type == SDL_QUIT)
                    {
                        quit = true;
                    }

                    //Tyhjennä näyttö
                    SDL_SetRenderDrawColor( gRenderer, 0xFF, 0xFF, 0xFF, 0xFF );
                    SDL_RenderClear( gRenderer );
                    
                    //Renderöi tausta
                    background.render( 0 , 0 );
                    
                    //Renderöi pelin käynnistyskuviot
                    ApilaKuvioRulla.render( 120 , 120 );
                    ApilaKuvioRulla.render( 350 , 120 );
                    ApilaKuvioRulla.render( 580 , 120 );
                    
                    //Renderöi voittavat kuviot
                    voittava_kuvio.render( 120 , 120 );
                    voittava_kuvio2.render( 350 , 120 );
                    voittava_kuvio3.render( 580 , 120 );
                    
                    //Renderöi napit
                    PelaaNappi.render( 580, 450 );
                    PanosNappi.render( 120, 450 );
                    
                    //Renderöi voiton määrän näytölle
                    voitto.render( 460 - voitto.getWidth(), 58 );
                    
                    //Renderöi panoksen määrän näytölle
                    panos.render( 565 - panos.getWidth(), 58 );
                    
                    //Renderöi saldon määrän näytölle
                    saldo.render( 705 - saldo.getWidth() , 58 );
                    
                    //Päivittää näytön
                    SDL_RenderPresent( gRenderer );
            
        
                                // Pelaa -napin painaminen
                                if (e.type == SDL_MOUSEBUTTONDOWN)
                                {
                                    //Pelaa -napin painamisen sijainti
                                   if(e.button.x <= 744 && e.button.x >= 580 && e.button.y <= 570 && e.button.y >= 450)
                                   {
                                       //Renderöi pohjaan painetun pelaa -napin
                                        PelaaNappiPainettu.render( 580, 450 );

                                       //Toistaa napin painamisen ääniefektin
                                        Mix_PlayChannel( -1, NapinPainallus, 0 );
                                       
                                       //Päivittää näytön
                                       SDL_RenderPresent( gRenderer );
                                       
                                    }
                                }
            
                                // Panos napin painaminen
                                if (e.type == SDL_MOUSEBUTTONDOWN)
                                {
                                    //Panos -napin painamisen sijainti
                                    if(e.button.x <= 284 && e.button.x >= 120 && e.button.y <= 570 && e.button.y >= 450)
                                    {
                                        //Renderöi pohjaan painetun panos -napin
                                        PanosNappiPainettu.render( 120, 450 );
                                        
                                        //Toistaa napin painamisen ääniefektin
                                        Mix_PlayChannel( -1, NapinPainallus, 0 );
                                        
                                        //Päivittää näytön
                                        SDL_RenderPresent( gRenderer );
                                    }
                                }
            
                                // Pelaa napin nostaminen
                                if (e.type == SDL_MOUSEBUTTONUP)
                                {
                                    //Pelaa -napin nostamisen sijainti
                                    if(e.button.x <= 744 && e.button.x >= 580 && e.button.y <= 570 && e.button.y >= 450)
                                    {
                                        //Laittaa pelin pyörimään, jos pelisaldo yli 0:n
                                        if (SaldoMaara > 0)
                                        {
                                        
                                        double YhteisVoittoKerroin;
                                            
                                        //Vähentää panoksen saldosta
                                        SaldoMaara = SaldoMaara - PanosMaara;
                                        
                                        //Alustaa osumien määrän 0:aan
                                        ApilaOsumat = 0;
                                        TahtiOsumat = 0;
                                        MansikkaOsumat = 0;
                                        MeloniOsumat = 0;
                                        BanaaniOsumat = 0;
                                        ViinirypaleOsumat = 0;
                                        
                                        //Alustaa kuvioiden kertoimet 0:aan
                                        ApilaVoittoKerroin = 0;
                                        TahtiVoittoKerroin = 0;
                                        MansikkaVoittoKerroin = 0;
                                        MeloniVoittoKerroin = 0;
                                        BanaaniVoittoKerroin = 0;
                                        ViinirypaleVoittoKerroin = 0;

                                        //Renderöi pyörivät rullat
                                        PyorivaRulla.render( 120 , 120 );
                                        PyorivaRulla.render( 350 , 120 );
                                        PyorivaRulla.render( 580 , 120 );
                        
                                        //Päivittää näytön
                                        SDL_RenderPresent( gRenderer );
                                         
                                        //Toistaa pyörivän rullan ääniefektin
                                        Mix_PlayChannel( -1, RullaPyorii, 0 );
                                            
                                        //Odottaa sekunnin
                                        SDL_Delay( 1000 );

                                        //Alustaa kertoimen 0:aan
                                        int kerroin = 0;
                                        
                                        //Arpoo sattumanvaraisen luvun 1-64 välillä, jossa aikaa käytetään siemenlukuna
                                        srand((int)time(0));
                                        kerroin = (rand()%64 + 1);
                                        
                                        //Näyttää konsolissa ensimmäisen kertoimen luvun
                                        cout << kerroin << endl;
                                        
                                        //Arvotusta tuloksesta riippuen lisää voiton tietyn kuvion osumamäärään sekä määrittää kuvion voittajaksi
                                        if (kerroin < 5) {
                                            voittava_kuvio = ApilaKuvioRulla;
                                            ApilaOsumat = ApilaOsumat + 1;
                                        }
                                        if (kerroin < 11 && kerroin >= 5 ) {
                                            voittava_kuvio = TahtiKuvioRulla;
                                            TahtiOsumat = TahtiOsumat + 1;
                                        }
                                        if (kerroin < 18 && kerroin >= 11 ) {
                                            voittava_kuvio = MansikkaKuvioRulla;
                                            MansikkaOsumat = MansikkaOsumat + 1;
                                        }
                                        if (kerroin < 30 && kerroin >= 18 ) {
                                            voittava_kuvio = MeloniKuvioRulla;
                                            MeloniOsumat = MeloniOsumat + 1;
                                        }
                                        if (kerroin < 45 && kerroin >= 30 ) {
                                            voittava_kuvio = BanaaniKuvioRulla;
                                            BanaaniOsumat = BanaaniOsumat + 1;
                                        }
                                        if (kerroin < 65 && kerroin >= 45 ) {
                                            voittava_kuvio = ViinirypaleKuvioRulla;
                                            ViinirypaleOsumat = ViinirypaleOsumat + 1;
                                        }
                                        
                                        //Renderöi näytölle voittavan kuvion
                                        voittava_kuvio.render( 120 , 120 );
                                        PyorivaRulla.render( 350 , 120 );
                                        PyorivaRulla.render( 580 , 120 );
                                        
                                        //Päivittää näytön
                                        SDL_RenderPresent( gRenderer );
                                        
                                        //Toistaa pyörivän rullan ja pysähtyvän rullan ääniefektin
                                        Mix_PlayChannel( -1, RullaPyorii, 0 );
                                        Mix_PlayChannel( -1, RullaPysahtyyChunk, 0 );
                                        
                                        //Odottaa sekunnin
                                        SDL_Delay( 1000 );

                                        //Alustaa toisen kertoimen 0:aan
                                        int kerroin2 = 0;
                                        
                                        //Arpoo sattumanvaraisen luvun 1-64 välillä
                                        kerroin2 = (rand()%64 + 1);
                                        
                                        //Näyttää konsolissa toisen kertoimen luvun
                                        cout << kerroin2 << endl;
                                        
                                        //Arvotusta tuloksesta riippuen lisää voiton tietyn kuvion osumamäärään sekä määrittää kuvion voittajaksi
                                        if (kerroin2 < 7) {
                                            voittava_kuvio2 = ApilaKuvioRulla;
                                            ApilaOsumat = ApilaOsumat + 1;
                                        }
                                        if (kerroin2 < 13 && kerroin2 >= 7 ) {
                                            voittava_kuvio2 = TahtiKuvioRulla;
                                            TahtiOsumat = TahtiOsumat + 1;
                                        }
                                        if (kerroin2 < 23 && kerroin2 >= 13 ) {
                                            voittava_kuvio2 = MansikkaKuvioRulla;
                                            MansikkaOsumat = MansikkaOsumat + 1;
                                        }
                                        if (kerroin2 < 38 && kerroin2 >= 23 ) {
                                            voittava_kuvio2 = MeloniKuvioRulla;
                                            MeloniOsumat = MeloniOsumat + 1;
                                        }
                                        if (kerroin2 < 58 && kerroin2 >= 38 ) {
                                            voittava_kuvio2 = BanaaniKuvioRulla;
                                            BanaaniOsumat = BanaaniOsumat + 1;
                                        }
                                        if (kerroin2 < 65 && kerroin2 >= 58 ) {
                                            voittava_kuvio2 = ViinirypaleKuvioRulla;
                                            ViinirypaleOsumat = ViinirypaleOsumat + 1;
                                        }
                                        
                                        //Renderöi näytölle voittavan kuvion
                                        voittava_kuvio.render( 120 , 120 );
                                        voittava_kuvio2.render( 350 , 120 );
                                        PyorivaRulla.render( 580 , 120 );
                                        
                                        //Päivittää näytön
                                        SDL_RenderPresent( gRenderer );
                                            
                                        //Toistaa pyörivän rullan ja pysähtyvän rullan ääniefektin
                                        Mix_PlayChannel( -1, RullaPyorii, 0 );
                                        Mix_PlayChannel( -1, RullaPysahtyyChunk, 0 );
                                            
                                        //Odottaa sekunnin
                                        SDL_Delay( 1000 );
                                            
                                        //Alustaa kolmannen kertoimen 0:aan
                                        int kerroin3 = 0;
                                       
                                        //Arpoo sattumanvaraisen luvun 1-64 välillä
                                        kerroin3 = (rand()%64 + 1);
                                        
                                        //Näyttää konsolissa kolmannen kertoimen luvun
                                        cout << kerroin3 << endl;
                                        
                                        //Arvotusta tuloksesta riippuen lisää voiton tietyn kuvion osumamäärään sekä määrittää kuvion voittajaksi
                                        if (kerroin3 < 7) {
                                            voittava_kuvio3 = ApilaKuvioRulla;
                                            ApilaOsumat = ApilaOsumat + 1;
                                        }
                                        if (kerroin3 < 13 && kerroin3 >= 7 ) {
                                            voittava_kuvio3 = TahtiKuvioRulla;
                                            TahtiOsumat = TahtiOsumat + 1;
                                        }
                                        if (kerroin3 < 23 && kerroin3 >= 13 ) {
                                            voittava_kuvio3 = MansikkaKuvioRulla;
                                            MansikkaOsumat = MansikkaOsumat + 1;
                                        }
                                        if (kerroin3 < 35 && kerroin3 >= 23 ) {
                                            voittava_kuvio3 = MeloniKuvioRulla;
                                            MeloniOsumat = MeloniOsumat + 1;
                                        }
                                        if (kerroin3 < 49 && kerroin3 >= 35 ) {
                                            voittava_kuvio3 = BanaaniKuvioRulla;
                                            BanaaniOsumat = BanaaniOsumat + 1;
                                        }
                                        if (kerroin3 < 65 && kerroin3 >= 49 ) {
                                            voittava_kuvio3 = ViinirypaleKuvioRulla;
                                            ViinirypaleOsumat = ViinirypaleOsumat + 1;
                                        }
                                        
                                        //Renderöi näytölle voittavan kuvion
                                        voittava_kuvio.render( 120 , 120 );
                                        voittava_kuvio2.render( 350 , 120 );
                                        voittava_kuvio3.render( 580 , 120 );
                                        
                                        //Päivittää näytön
                                        SDL_RenderPresent( gRenderer );
                                            
                                        //Toistaa pysähtyvän rullan ääniefektin
                                        Mix_PlayChannel( -1, RullaPysahtyyChunk, 0 );
                                        
                                        //Määrittää voittokertoimen osumille
                                        // Apilaosumat
                                        if (ApilaOsumat == 1) {
                                            ApilaVoittoKerroin = 1;
                                        }
                                        if (ApilaOsumat == 2) {
                                            ApilaVoittoKerroin = 10;
                                        }
                                        if (ApilaOsumat == 3) {
                                            ApilaVoittoKerroin = 100;
                                        }
                                        
                                        // Tähtiosumat
                                        if (TahtiOsumat == 1) {
                                            TahtiVoittoKerroin = 0.5;
                                        }
                                        if (TahtiOsumat == 2) {
                                            TahtiVoittoKerroin = 5;
                                        }
                                        if (TahtiOsumat == 3) {
                                            TahtiVoittoKerroin = 50;
                                        }
                                        
                                        // Mansikkaosumat
                                        if (MansikkaOsumat == 1) {
                                            MansikkaVoittoKerroin = 0;
                                        }
                                        if (MansikkaOsumat == 2) {
                                            MansikkaVoittoKerroin = 2.5;
                                        }
                                        if (MansikkaOsumat == 3) {
                                            MansikkaVoittoKerroin = 25;
                                        }
                                        
                                        // Meloniosumat
                                        if (MeloniOsumat == 1) {
                                            MeloniVoittoKerroin = 0;
                                        }
                                        if (MeloniOsumat == 2) {
                                            MeloniVoittoKerroin = 1.25;
                                        }
                                        if (MeloniOsumat == 3) {
                                            MeloniVoittoKerroin = 12.5;
                                        }
                                        
                                        // Banaaniosumat
                                        if (BanaaniOsumat == 1) {
                                            BanaaniVoittoKerroin = 0;
                                        }
                                        if (BanaaniOsumat == 2) {
                                            BanaaniVoittoKerroin = 0;
                                        }
                                        if (BanaaniOsumat == 3) {
                                            BanaaniVoittoKerroin = 6.25;
                                        }
                                        
                                        // Viinirypäleosumat
                                        if (ViinirypaleOsumat == 1) {
                                            ViinirypaleVoittoKerroin = 0;
                                        }
                                        if (ViinirypaleOsumat == 2) {
                                            ViinirypaleVoittoKerroin = 0;
                                        }
                                        if (ViinirypaleOsumat == 3) {
                                            ViinirypaleVoittoKerroin = 3.25;
                                        }
                                        
                                        //Laskee yhteen voittokertoimet
                                        YhteisVoittoKerroin = ApilaVoittoKerroin + TahtiVoittoKerroin + MansikkaVoittoKerroin + MeloniVoittoKerroin + BanaaniVoittoKerroin + ViinirypaleVoittoKerroin;
                                        
                                        //Kertoo voittokertoimen panoksella
                                        VoittoMaara = YhteisVoittoKerroin * PanosMaara;
                                        
                                        //Lisää voiton pelisaldoon
                                        SaldoMaara = SaldoMaara + VoittoMaara;
                                        
                                        cout << YhteisVoittoKerroin << endl;
                                        
                                        cout << VoittoMaara << endl;
                                        
                                        //Muuttaa kokonaisluvun merkkijonoksi
                                        std::string out_string;
                                        std::stringstream ss;
                                        ss << VoittoMaara;
                                        out_string = ss.str();
                                        
                                        //Renderöi voiton määrä -tekstin
                                        SDL_Color textColor = { 255, 0, 0 };
                                        voitto.loadFromRenderedText( ss.str(), textColor );
                                        
                                        //Muuttaa kokonaisluvun merkkijonoksi
                                        std::string out_string3;
                                        std::stringstream ss3;
                                        ss3 << SaldoMaara;
                                        out_string = ss3.str();
                                        
                                        //Renderöi saldon määrä -tekstin
                                        saldo.loadFromRenderedText( ss3.str(), textColor );
                                            
                                            //Toistaa voiton ääniefektin, kun pelistä tulee voitto
                                            if (YhteisVoittoKerroin > 0) {
                                                Mix_PlayChannel( -1, PieniVoitto, 0);
                                            }
                                        }
                                    }
                                }
    
                                // Panos -napin nostaminen
                                if (e.type == SDL_MOUSEBUTTONUP)
                                {
                                    //Panos -napin painamisen sijainti
                                    if(e.button.x <= 284 && e.button.x >= 120 && e.button.y <= 570 && e.button.y >= 450)
                                    {
                                        //Nostaa panosta 0.2 yksikköä nappia painettaessa maksimipanos 2.0:aan asti, jonka jälkeen panos on taas 0.2
                                        PanosMaara += 0.2;
                                        if (PanosMaara > 2.0) {
                                            PanosMaara = 0.2;
                                            cout << PanosMaara << endl;
                                        }
                                        
                                        //Muuttaa kokonaisluvun merkkijonoksi
                                        cout << PanosMaara << endl;
                                        std::string out_string2;
                                        std::stringstream ss2;
                                        ss2 << PanosMaara;
                                        out_string2 = ss2.str();
                                        
                                        //Renderöi panostekstin
                                        SDL_Color textColor = { 255, 0, 0 };
                                        panos.loadFromRenderedText( ss2.str(), textColor );
                                    }
                                }
                }
            }
        }
    }
    
    //Vapauttaa resurssit ja sulkee SDL:län
    close();
    
    return 0;
}
