#include "nextPuyo.h"

namespace ppvs
{

nextPuyo::nextPuyo()
{
    //Nextpuyo window is 65 pixels wide, 124 pixels high
    gamedata=0;
    m_scale=1;
    m_goNext=16;
    m_color11=0; m_color12=0;
    m_color21=0; m_color22=0;
    m_color31=0; m_color32=0;
    resetPuyoPos();
    m_offsetX=0; m_offsetY=0;
    m_orientation=true; //orientation sets nextpuyo window left or right
    m_sign=1;
    initialize=false;
}

nextPuyo::~nextPuyo()
{
    //dtor
}

int nextPuyo::getOrientation()
{
    return m_sign;
}

void nextPuyo::init(float x,float y,float scale,bool orientation,gameData* globalp)
{
    gamedata=globalp;
    // !!!!!!!!!!!!!!
    //nextPuyoRenderImage.set(&m_final,global->AppWindow);
    // !!!!!!!!!!!!!!
    initImage();
    m_offsetX=x; m_offsetY=y;
    m_scale=scale;
    m_orientation=orientation;
    m_sign=int(orientation)*2-1;
    if (initialize)
        return;

    if (orientation)
    {
        if (gamedata)
        m_background.setImage(gamedata->imgNextPuyoBackgroundR);
        //m_cutSprite.setImage(gamedata->imgCutR);
    }
    else
    {
        if (gamedata)
        m_background.setImage(gamedata->imgNextPuyoBackgroundL);
        //m_cutSprite.setImage(gamedata->imgCutL);
    }

    m_background.setSubRect(0,0,65,124);

    //nextPuyoRenderImage.create(m_background.getSizeX(),m_background.getSizeY());

    initialize=true;
    resetPuyoPos();
    m_sprite31.setScale(33.0/48.0);
    m_sprite32.setScale(33.0/48.0);
}

void nextPuyo::initImage()
{
    if (gamedata)
    {
        m_sprite11.setImage(gamedata->imgPuyo);m_sprite12.setImage(gamedata->imgPuyo);
        m_sprite21.setImage(gamedata->imgPuyo);m_sprite22.setImage(gamedata->imgPuyo);
        m_sprite31.setImage(gamedata->imgPuyo);m_sprite32.setImage(gamedata->imgPuyo);
        m_eye11.setImage(gamedata->imgPuyo);m_eye12.setImage(gamedata->imgPuyo);
        m_eye21.setImage(gamedata->imgPuyo);m_eye22.setImage(gamedata->imgPuyo);
        m_eye31.setImage(gamedata->imgPuyo);m_eye32.setImage(gamedata->imgPuyo);
    }

    movePuyoType mpt=DOUBLET;
    setSprite(m_sprite11,m_sprite12,m_eye11,m_eye12,m_color11,m_color12,mpt);
    setSprite(m_sprite21,m_sprite22,m_eye21,m_eye22,m_color21,m_color22,mpt);
    setSprite(m_sprite31,m_sprite32,m_eye31,m_eye32,m_color31,m_color32,mpt);

    m_cutSprite.setBlendMode(multiplyBlending);

}

void nextPuyo::setPuyo()
{//set correct sprites
    setSprite(m_sprite11,m_sprite12,m_eye11,m_eye12,m_color11,m_color12,m_type1);
    setSprite(m_sprite21,m_sprite22,m_eye21,m_eye22,m_color21,m_color22,m_type2);
    setSprite(m_sprite31,m_sprite32,m_eye31,m_eye32,m_color31,m_color32,m_type3);
}

void nextPuyo::play()
{//move nextpuyo
    if (m_goNext>=16 && m_pair1Y>0)
        m_pair1Y-=4;
    if (m_goNext<16)
    {
        m_pair1Y-=6;
        m_pair3Y-=4;
        m_pair2X-=22.0/16.0*m_sign;
        m_pair2Y-=56.0/16.0;

        m_sprite21.setScale((33.0+m_goNext)/48.0);
        m_sprite22.setScale((33.0+m_goNext)/48.0);

        ++m_goNext;
    }
}

void nextPuyo::update(std::deque<int> &dq,puyoCharacter p,int turn)
{//set colors of puyo to player's deque
    resetPuyoPos();
    m_goNext=0;

    //set colors and call setpuyo
    //the list must be at least 6 big
    if (dq.size()>=6)
    {
        m_color11=*(dq.begin()); m_color12=*(dq.begin()+1);
        m_color21=*(dq.begin()+2); m_color22=*(dq.begin()+3);
        m_color31=*(dq.begin()+4); m_color32=*(dq.begin()+5);
        m_type1=getFromDropPattern(p,turn);
        m_type2=getFromDropPattern(p,turn+1);
        m_type3=getFromDropPattern(p,turn+2);
        setPuyo();
    }
    else
    {//list wasn't initialized -> set invisible?

    }
}

void nextPuyo::draw()
{
    //Set positions of sprites
    m_sprite11.setPosition(m_pair1X,m_pair1Y);m_sprite12.setPosition(m_pair1X,m_pair1Y);
    m_sprite21.setPosition(m_pair2X,m_pair2Y);m_sprite22.setPosition(m_pair2X,m_pair2Y);
    m_sprite31.setPosition(m_pair3X,m_pair3Y);m_sprite32.setPosition(m_pair3X,m_pair3Y);
    m_eye11.setPosition(m_pair1X,m_pair1Y);m_eye12.setPosition(m_pair1X,m_pair1Y);
    m_eye21.setPosition(m_pair2X,m_pair2Y);m_eye22.setPosition(m_pair2X,m_pair2Y);
    m_eye31.setPosition(m_pair3X,m_pair3Y);m_eye32.setPosition(m_pair3X,m_pair3Y);
    //Copy background (consider copying image without alpha channel)
    if (!gamedata)
        return;
    gamedata->front->clearDepth();
    gamedata->front->setDepthFunction(lessOrEqual);
    gamedata->front->enableAlphaTesting(0.1f);
    m_background.draw(gamedata->front);
    //draw puyos
    gamedata->front->setDepthFunction(equal);
    m_sprite11.draw(gamedata->front);
    m_sprite12.draw(gamedata->front);
    m_sprite21.draw(gamedata->front);
    m_sprite22.draw(gamedata->front);
    m_sprite31.draw(gamedata->front);
    m_sprite32.draw(gamedata->front);

    //Set eyes
    m_eye21.setScale(m_sprite21.getScaleX());
    m_eye22.setScale(m_sprite22.getScaleX());
    m_eye31.setScale(m_sprite31.getScaleX());
    m_eye32.setScale(m_sprite32.getScaleX());

    m_eye11.draw(gamedata->front);
    m_eye12.draw(gamedata->front);
    m_eye21.draw(gamedata->front);
    m_eye22.draw(gamedata->front);
    m_eye31.draw(gamedata->front);
    m_eye32.draw(gamedata->front);
    gamedata->front->setDepthFunction(always);
    gamedata->front->disableAlphaTesting();

    //draw final
    /*
    nextPuyoRenderImage.display();
    m_final.setImage(nextPuyoRenderImage.getImage());
    m_final.setPosition(m_offsetX,m_offsetY);
    m_final.setScale(m_scale,m_scale);
    m_final.draw(global->AppWindow);
    */
//
//    if (!m_orientation)
//    m_cutSprite.setBlendMode(sf::Blend::None);
//    m_cutSprite.draw(global->AppWindow);

}

void nextPuyo::setSprite(sprite &m_sprite1,sprite &m_sprite2,sprite &m_spriteEye1,sprite &m_spriteEye2,int &m_color1,int &m_color2,movePuyoType &m_type)
{//it's almost a direct copy of movepuyo.cpp. Difference in centers
    float subRectX1,subRectY1,subRectWidth1,subRectHeight1,subRectX2,subRectY2,subRectWidth2,subRectHeight2;
    int m_bigColor=m_color1;
    if (m_type==DOUBLET)
    {
        subRectX1=0; subRectY1=PUYOY*m_color1;
        subRectWidth1=PUYOX; subRectHeight1=PUYOY;
        subRectX2=0; subRectY2=PUYOY*m_color2;
        subRectWidth2=PUYOX; subRectHeight2=PUYOY;
        m_sprite1.setSubRect(subRectX1,subRectY1,subRectWidth1,subRectHeight1);
        m_sprite2.setSubRect(subRectX2,subRectY2,subRectWidth2,subRectHeight2);
        m_sprite1.setCenter(PUYOX/2,0);
        m_sprite2.setCenter(PUYOX/2,PUYOY);
        m_sprite1.setVisible(true);
        m_sprite2.setVisible(true);
        m_spriteEye1.setVisible(false);
        m_spriteEye2.setVisible(false);
    }
    else if (m_type==TRIPLET && m_color1!=m_color2)
    {
        subRectX1=PUYOX*m_color1; subRectY1=PUYOY*5+1;
        subRectWidth1=PUYOX; subRectHeight1=PUYOY*2-1;
        subRectX2=0; subRectY2=PUYOY*m_color2;
        subRectWidth2=PUYOX; subRectHeight2=PUYOY;
        m_sprite1.setSubRect(subRectX1,subRectY1,subRectWidth1,subRectHeight1);
        m_sprite2.setSubRect(subRectX2,subRectY2,subRectWidth2,subRectHeight2);
        m_sprite1.setCenter(PUYOX,PUYOY);
        m_sprite2.setCenter(0,0);
        m_sprite1.setVisible(true);
        m_sprite2.setVisible(true);
        m_spriteEye1.setSubRect(PUYOX+2*PUYOX*((m_color1/2)),12*PUYOY+PUYOY*(m_color1%2),PUYOX,PUYOY);
        m_spriteEye2.setSubRect(PUYOX+2*PUYOX*((m_color2/2)),12*PUYOY+PUYOY*(m_color2%2),PUYOX,PUYOY);
        m_spriteEye1.setCenter(PUYOX,PUYOY);
        m_spriteEye2.setCenter(0,0);
        m_spriteEye1.setVisible(true);
        m_spriteEye2.setVisible(false);
    }
    else if ((m_type==TRIPLET || m_type==TRIPLETR) && m_color1==m_color2)
    {
        subRectX1=5*PUYOX+2*PUYOX*m_color1; subRectY1=PUYOY*5+1;
        subRectWidth1=PUYOX*2; subRectHeight1=PUYOY*2-1;
        subRectX2=0; subRectY2=PUYOY*m_color2;
        subRectWidth2=PUYOX; subRectHeight2=PUYOY;
        m_sprite1.setSubRect(subRectX1,subRectY1,subRectWidth1,subRectHeight1);
        m_sprite2.setSubRect(subRectX2,subRectY2,subRectWidth2,subRectHeight2);
        m_sprite1.setCenter(PUYOX,PUYOY);
        m_sprite2.setCenter(0,0);
        m_sprite1.setVisible(true);
        m_sprite2.setVisible(false);
        m_spriteEye1.setSubRect(PUYOX+2*PUYOX*((m_color1/2)),12*PUYOY+PUYOY*(m_color1%2),PUYOX,PUYOY);
        m_spriteEye2.setSubRect(PUYOX+2*PUYOX*((m_color2/2)),12*PUYOY+PUYOY*(m_color2%2),PUYOX,PUYOY);
        m_spriteEye1.setCenter(PUYOX,PUYOY);
        m_spriteEye2.setCenter(0,0);
        m_spriteEye1.setVisible(true);
        m_spriteEye2.setVisible(false);
    }
    else if (m_type==TRIPLETR && m_color1!=m_color2)
    {
        subRectX1=PUYOX*m_color1; subRectY1=PUYOY*5+1;
        subRectWidth1=PUYOX; subRectHeight1=PUYOY*2-1;
        subRectX2=0; subRectY2=PUYOY*m_color2;
        subRectWidth2=PUYOX; subRectHeight2=PUYOY;
        m_sprite1.setSubRect(subRectX1,subRectY1,subRectWidth1,subRectHeight1);
        m_sprite2.setSubRect(subRectX2,subRectY2,subRectWidth2,subRectHeight2);
        m_sprite1.setCenter(0,PUYOY);
        m_sprite2.setCenter(PUYOX,PUYOY);
        m_sprite1.setVisible(true);
        m_sprite2.setVisible(true);
        m_spriteEye1.setSubRect(PUYOX+2*PUYOX*((m_color1/2)),12*PUYOY+PUYOY*(m_color1%2),PUYOX,PUYOY);
        m_spriteEye2.setSubRect(PUYOX+2*PUYOX*((m_color2/2)),12*PUYOY+PUYOY*(m_color2%2),PUYOX,PUYOY);
        m_spriteEye1.setCenter(0,0);
        m_spriteEye2.setCenter(PUYOX,PUYOY);
        m_spriteEye1.setVisible(true);
        m_spriteEye2.setVisible(false);

    }
    else if (m_type==QUADRUPLET)
    {
        subRectX1=10*PUYOX+(PUYOX+PUYOX/6.4f)*m_color1; subRectY1=PUYOY*13;
        subRectWidth1=PUYOX+PUYOX/6.4f; subRectHeight1=PUYOY*2;
        subRectX2=10*PUYOX+(PUYOX+PUYOX/6.4f)*m_color2; subRectY2=PUYOY*13;
        subRectWidth2=PUYOX+PUYOX/6.4f; subRectHeight2=PUYOY*2;
        m_sprite2.setSubRect(subRectX1,subRectY1,subRectWidth1,subRectHeight1);
        m_sprite1.setSubRect(subRectX2,subRectY2,subRectWidth2,subRectHeight2);
        if (gamedata)
        {
            m_sprite2.setCenter(gamedata->PUYOXCENTER,PUYOY);
            m_sprite1.setCenter(gamedata->PUYOXCENTER,PUYOY);
        }
        //m_sprite1.setCenter(PUYOX*0.91f,PUYOY*0.84f);
        //m_sprite2.setCenter(PUYOX*0.91f,PUYOY*0.84f);
        m_sprite1.setVisible(true);
        m_sprite2.setVisible(true);
        m_spriteEye1.setSubRect(PUYOX+2*PUYOX*((m_color1/2)),12*PUYOY+PUYOY*(m_color1%2),PUYOX,PUYOY);
        m_spriteEye2.setSubRect(PUYOX+2*PUYOX*((m_color2/2)),12*PUYOY+PUYOY*(m_color2%2),PUYOX,PUYOY);
        m_spriteEye2.setCenter(PUYOX-10,PUYOY-4);
        m_spriteEye1.setCenter(0+10,0+4);
        m_spriteEye1.setVisible(true);
        m_spriteEye2.setVisible(true);
    }
    else if (m_type==BIG)
    {
        subRectX1=2*PUYOX+2*PUYOX*m_bigColor; subRectY1=PUYOY*7;
        subRectWidth1=PUYOX*2; subRectHeight1=PUYOY*2;
        subRectX2=0; subRectY2=0;
        subRectWidth2=0; subRectHeight2=0;
        m_sprite1.setSubRect(subRectX1,subRectY1,subRectWidth1,subRectHeight1);
        m_sprite2.setSubRect(subRectX2,subRectY2,subRectWidth2,subRectHeight2);
        m_sprite1.setCenter(PUYOX,PUYOY);
        m_sprite2.setCenter(PUYOX,PUYOY);
        m_sprite1.setVisible(true);
        m_sprite2.setVisible(false);
        m_spriteEye1.setVisible(false);
        m_spriteEye2.setVisible(false);
    }
    if (m_type==TRIPLETR && m_color1!=m_color2)
    {
        m_sprite1.setRotation(-90);
        m_sprite2.setRotation(0);
    }
    else if (m_type==QUADRUPLET)
    {
        m_sprite1.setRotation(0);
        m_sprite2.setRotation(180);
    }
    else
    {
        m_sprite1.setRotation(0);
        m_sprite2.setRotation(0);
    }
}

void nextPuyo::idle()
{//reset position when not playing
    m_goNext=0;
    resetPuyoPos();
}

void nextPuyo::test()
{
    m_goNext=0;
    resetPuyoPos();
}

void nextPuyo::resetPuyoPos()
{
    m_pair1X=int(!m_orientation)*65+m_sign*25; m_pair1Y=40;
    m_pair2X=int(!m_orientation)*65+m_sign*47; m_pair2Y=96;
    m_pair3X=int(!m_orientation)*65+m_sign*47; m_pair3Y=160;
    m_sprite21.setScale(33.0/48.0);
    m_sprite22.setScale(33.0/48.0);
}
}
