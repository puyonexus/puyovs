#include "animation.h"

namespace ppvs
{

animation::animation()
{
    //ctor
    t=1000;
    duration=120;
    error=true;
//    offset.x=0; offset.y=0;
//    globalScale=0;
//    sourceFolder="";
}

animation::~animation()
{
    //dtor
    clearSprites();
    clearSounds();
}

void animation::init(gameData *g,posVectorFloat offsetPos,float scale,std::string folder,std::string scriptName,int maxTime)
{
//    currentCharacter=m_player->currentgame->settings->characterSetup[pl->getCharacter()];
//    currentCharacter="Amitie";
//    std::string scriptName=folder_user_character+currentCharacter+std::string("/Animation/")+scriptName;
//    bool loadOK=doc.LoadFile(scriptName);
    gamedata=g;
    clearSprites();
    paths.clear();
    sourceFolder=folder;
    offset=offsetPos;
    globalScale=scale;
    duration=maxTime;
    error=false; //--> in case script goes wrong
    //load scripts
    bool loadOK=doc.LoadFile(sourceFolder+scriptName);
    if (!loadOK)
    {
        error=true;
        std::cout<<sourceFolder+scriptName+" could not be loaded\n"<<std::endl;
        return;
    }
    //initialize sprites etc.
    TiXmlHandle docHandle( &doc );
    //find init
    TiXmlElement* init = docHandle.FirstChild("animations").FirstChild( "init" ).ToElement();
    if (init)
    {
        //traverse through init's inner elements
        TiXmlNode* init_child = 0;
        while((init_child = init->IterateChildren(init_child)) != 0)
        {
            //cout<<"Found child "<<init_child->Value()<<": "<<init_child->ToElement()->Attribute("name")<<endl;
            if (init_child->ValueStr()=="sprite")
            {
                std::string spritename;
                //get sprite name
                if (init_child->ToElement()->Attribute("name"))
                {
                    spritename=init_child->ToElement()->Attribute("name");
                }
                else
                {//cannot find spritename
                    continue; //must have a name!
                }

                //check if before something else
                std::string before="";
                if (init_child->ToElement()->Attribute("before"))
                {
                    before=init_child->ToElement()->Attribute("before");
                }

                //traverse through sprite's elements
                TiXmlNode* sprite_child = 0;
                while((sprite_child = init_child->IterateChildren(sprite_child)) != 0)
                {
                    //get image name
                    std::string imagename;
                    if (sprite_child->ValueStr()=="image")
                    {
                        imagename=sprite_child->ToElement()->GetText();
                        //now we can add the sprite
                        newSprite(spritename,imagename,before);
                    }
                    //get subrectangle
                    if (sprite_child->ValueStr()=="rect")
                    {//find x,y,widht and height attributes.
                        int x,y,width,height;
                        if (sprite_child->ToElement()->Attribute("x",&x) &&
                            sprite_child->ToElement()->Attribute("y",&y) &&
                            sprite_child->ToElement()->Attribute("width",&width) &&
                            sprite_child->ToElement()->Attribute("height",&height))
                            setRect(spritename,x,y,width,height);
                    }
                    //get center
                    if (sprite_child->ValueStr()=="center")
                    {//find x,y,widht and height attributes.
                        int x,y;
                        if (sprite_child->ToElement()->Attribute("x",&x) &&
                            sprite_child->ToElement()->Attribute("y",&y))
                            setCenter(spritename,x,y);
                    }
                    //boundsprite
                    if (sprite_child->ValueStr()=="bind")
                    {//find x,y,widht and height attributes.
                        std::string parent;
                        int x,y;
                        if (sprite_child->ToElement()->Attribute("parent") &&
                            sprite_child->ToElement()->Attribute("x",&x) &&
                            sprite_child->ToElement()->Attribute("y",&y))
                        {
                            parent=sprite_child->ToElement()->Attribute("parent");
                            //set parent and add self to parent as child
                            sprites[spritename].parent=parent;
                            addChild(parent,spritename,x,y);
                            //setPosition(spritename,x,y);
                        }
                    }
                    if (sprite_child->ValueStr()=="blend")
                    {
                        std::string mode;
                        if (sprite_child->ToElement()->Attribute("mode"))
                        {
                            mode=sprite_child->ToElement()->Attribute("mode");
                            setBlendmode(spritename,mode);
                        }
                    }

                }
            }
            else if (init_child->ValueStr()=="path")
            {
                std::string pathname;
                //get path name
                if (init_child->ToElement()->Attribute("name"))
                {
                    pathname=init_child->ToElement()->Attribute("name");
                }
                else
                {//cannot find name
                    continue; //must have a name!
                }
                //traverse through path's elements
                TiXmlNode* path_child = 0;
                while((path_child = init_child->IterateChildren(path_child)) != 0)
                {
                    //add nodes
                    if (path_child->ValueStr()=="node")
                    {
                        int x,y;
                        if (path_child->ToElement()->Attribute("x",&x) &&
                            path_child->ToElement()->Attribute("y",&y))
                        {
                            //add node
                            posVectorFloat pv(x,y);
                            paths[pathname].push_back(pv);
                        }
                    }
                }
            }
            else if (init_child->ValueStr()=="sound")
            {
                std::string spritename;
                //get sprite name
                if (init_child->ToElement()->Attribute("name"))
                {
                    spritename=init_child->ToElement()->Attribute("name");
                }
                else
                {//cannot find spritename
                    continue; //must have a name!
                }
                //traverse through sprite's elements
                TiXmlNode* sprite_child = 0;
                while((sprite_child = init_child->IterateChildren(sprite_child)) != 0)
                {
                    //get file name
                    std::string imagename;
                    if (sprite_child->ValueStr()=="file")
                    {
                        imagename=sprite_child->ToElement()->GetText();
                        //now we can add the sprite
                        newSound(spritename,imagename);
                    }
                }
            }

        }
    }
    else
    {
        error=true;
    }
}

void animation::playAnimation()
{
    if (error)
        return;

    //the timer t runs from 0 to 120 (2 seconds)
    if (t<=duration)
    {
        //animationName must be set beforehand
        if (animationName=="")
            return;

        TiXmlHandle docHandle( &doc );
        //find animation element
        TiXmlElement* anim = docHandle.FirstChild("animations").FirstChild(animationName.c_str()).ToElement();
        if (anim)
        {
            //traverse through keyframes
            TiXmlNode* anim_child = 0;
            while((anim_child = anim->IterateChildren(anim_child)) != 0)
            {
                if (anim_child->ValueStr()=="event")
                {
                    //get start time
                    int starttime;
                    if (anim_child->ToElement()->Attribute("start",&starttime))
                    {
                        //check if keyframe is started
                        if (starttime>t)
                            continue;
                    }
                    else
                    {//erroneous keyframe element: start time must be defined
                        continue;
                    }

                    //traverse through keyfames's elements
                    TiXmlNode* kf_child = 0;
                    while((kf_child = anim_child->IterateChildren(kf_child)) != 0)
                    {
                        //play sound
                        if (starttime==t)
                        if (kf_child->ValueStr()=="playSound")
                        {//find x,y
                            std::string soundName;
                            if (kf_child->ToElement()->Attribute("name"))
                            {
                                soundName=kf_child->ToElement()->Attribute("name");
                                playSound(soundName);
                            }
                        }
                        //set time loop point
                        if (kf_child->ValueStr()=="loop")
                        {
                            int time=t;
                            if (kf_child->ToElement()->Attribute("t",&time))
                                t=time;
                        }


                        //get spritename
                        std::string spritename;
                        if (kf_child->ToElement()->Attribute("name"))
                        {
                            spritename=kf_child->ToElement()->Attribute("name");
                        }
                        else
                        {//all animation functions must point to a sprite name!
                            continue;
                        }

                        //Instantly change sprite property
                        //set subrectangle
                        if (kf_child->ValueStr()=="rect")
                        {//find x,y,widht and height attributes.
                            int x,y,width,height;
                            if (kf_child->ToElement()->Attribute("x",&x) &&
                                kf_child->ToElement()->Attribute("y",&y) &&
                                kf_child->ToElement()->Attribute("width",&width) &&
                                kf_child->ToElement()->Attribute("height",&height))
                                setRect(spritename,x,y,width,height);
                        }
                        //set center
                        if (kf_child->ValueStr()=="center")
                        {//find x,y,widht and height attributes.
                            int x,y;
                            if (kf_child->ToElement()->Attribute("x",&x) &&
                                kf_child->ToElement()->Attribute("y",&y))
                                setCenter(spritename,x,y);
                        }
                        //set position
                        if (kf_child->ValueStr()=="position")
                        {//find x,y
                            int x,y;
                            if (kf_child->ToElement()->Attribute("x",&x) &&
                                kf_child->ToElement()->Attribute("y",&y))
                                setPosition(spritename,x,y);
                        }
                        if (kf_child->ValueStr()=="blend")
                        {//find x,y
                            std::string mode;
                            if (kf_child->ToElement()->Attribute("mode"))
                            {
                                mode=kf_child->ToElement()->Attribute("mode");
                                setBlendmode(spritename,mode);
                            }
                        }


                        //is animation ready to be played?
                        int duration;
                        if (kf_child->ToElement()->Attribute("duration",&duration))
                        {
                            if (t>starttime+duration)
                                continue; //time has passed
                        }
                        else
                        {//duration of a function must be defined!
                            continue;
                        }
                        //get type
                        std::string type;
                        if (kf_child->ToElement()->Attribute("type"))
                        {
                            type=kf_child->ToElement()->Attribute("type");
                        }
                        else //default type
                            type="none";

                        //==Animation
                        //transform scale
                        if (kf_child->ValueStr()=="scaleX")
                        {
                            double startVal,endVal,alpha,beta;
                            if (!kf_child->ToElement()->Attribute("alpha",&alpha))alpha=0;
                            if (!kf_child->ToElement()->Attribute("beta",&beta))beta=0;
                            if (kf_child->ToElement()->Attribute("startVal",&startVal) &&
                                kf_child->ToElement()->Attribute("endVal",&endVal))
                                setScaleX(spritename,getLocalTimer(type,startVal,endVal,(t-starttime)*1.0/duration,alpha,beta));
                        }
                        if (kf_child->ValueStr()=="scaleY")
                        {
                            double startVal,endVal,alpha,beta;
                            if (!kf_child->ToElement()->Attribute("alpha",&alpha))alpha=0;
                            if (!kf_child->ToElement()->Attribute("beta",&beta))beta=0;
                            if (kf_child->ToElement()->Attribute("startVal",&startVal) &&
                                kf_child->ToElement()->Attribute("endVal",&endVal))
                                setScaleY(spritename,getLocalTimer(type,startVal,endVal,(t-starttime)*1.0/duration,alpha,beta));
                        }
                        //set transparency
                        if (kf_child->ValueStr()=="transparency")
                        {
                            double startVal,endVal,alpha,beta;
                            if (!kf_child->ToElement()->Attribute("alpha",&alpha))alpha=0;
                            if (!kf_child->ToElement()->Attribute("beta",&beta))beta=0;
                            if (kf_child->ToElement()->Attribute("startVal",&startVal) &&
                                kf_child->ToElement()->Attribute("endVal",&endVal))
                                setTransparency(spritename,getLocalTimer(type,startVal,endVal,(t-starttime)*1.0/duration,alpha,beta));
                        }
                        if (kf_child->ValueStr()=="rotation")
                        {
                            double startVal,endVal,alpha,beta;
                            if (!kf_child->ToElement()->Attribute("alpha",&alpha))alpha=0;
                            if (!kf_child->ToElement()->Attribute("beta",&beta))beta=0;
                            if (kf_child->ToElement()->Attribute("startVal",&startVal) &&
                                kf_child->ToElement()->Attribute("endVal",&endVal))
                                setAngle(spritename,getLocalTimer(type,startVal,endVal,(t-starttime)*1.0/duration,alpha,beta));
                        }
                        if (kf_child->ValueStr()=="move")
                        {
                            std::string pathname;
                            double startVal,endVal,alpha,beta;
                            if (!kf_child->ToElement()->Attribute("alpha",&alpha))alpha=0;
                            if (!kf_child->ToElement()->Attribute("beta",&beta))beta=0;
                            if (kf_child->ToElement()->Attribute("startVal",&startVal) &&
                                kf_child->ToElement()->Attribute("endVal",&endVal) &&
                                kf_child->ToElement()->Attribute("path"))
                            {
                                pathname=kf_child->ToElement()->Attribute("path");
                                move(spritename,pathname,getLocalTimer(type,startVal,endVal,(t-starttime)*1.0/duration,alpha,beta));
                            }
                        }
                        //color
                        if (kf_child->ValueStr()=="color")
                        {
                            std::string rgb;
                            double startVal,endVal,alpha,beta;
                            if (!kf_child->ToElement()->Attribute("alpha",&alpha))alpha=0;
                            if (!kf_child->ToElement()->Attribute("beta",&beta))beta=0;
                            if (kf_child->ToElement()->Attribute("startVal",&startVal) &&
                                kf_child->ToElement()->Attribute("endVal",&endVal)&&
                                kf_child->ToElement()->Attribute("rgb"))
                            {
                                rgb=kf_child->ToElement()->Attribute("rgb");
                                //split into two
                                setColor(spritename,rgb,getLocalTimer(type,startVal,endVal,(t-starttime)*1.0/duration,alpha,beta));
                            }
                        }
                    }
                }
            }
            t++;
        }
    }
    if (t>duration && t<900)
    {//end animation: hide all sprites
        std::map<std::string,animationSprite>::iterator it;
        for (it=sprites.begin();it!= sprites.end();it++)
        {
            (*it).second.transparency=0;
        }
        t=1000;
    }
}

void animation::draw()
{//draw all sprites
    updateSprites();
    std::list<animationSprite*>::iterator it;
    for (it=drawSprites.begin();it!= drawSprites.end();++it)
    {
        (**it).spr->draw(gamedata->front);
    }
}

void animation::newSprite(std::string &name,std::string &image,std::string &before)
{//define new sprite
    sprites[name].spr=new sprite();
    sprites[name].spr->setImage(gamedata->front->loadImage(sourceFolder+image));
    gamedata->front->loadImage(sourceFolder+image)->setFilter(linearFilter);
    sprites[name].angle=0;
    sprites[name].transparency=0;
    sprites[name].position.x=0;
    sprites[name].position.y=0;
    sprites[name].pathPos.x=0;
    sprites[name].pathPos.y=0;
    sprites[name].scale.x=1;
    sprites[name].scale.y=1;
    sprites[name].childOffset.x=0;
    sprites[name].childOffset.y=0;
    sprites[name].parent="";

    if (before=="")
    {//add sprite at the end
        drawSprites.push_back(&sprites[name]);
    }
    else
    {//find other sprite and insert at that position
        drawSprites.insert(std::find(drawSprites.begin(),drawSprites.end(),&sprites[before]),&sprites[name]);
    }

}
void animation::newSound(std::string &name,std::string &buffer)
{
    sounds[name]=new sound;
    setBuffer(*sounds[name],gamedata->front->loadSound(sourceFolder+buffer));
}
void animation::addChild(std::string &parent,std::string &child,float x,float y)
{//add child to parent
    sprites[parent].children.push_back(child);
    //child's transparency is 1 by default
    sprites[child].transparency=1;
    sprites[child].childOffset.x=x;
    sprites[child].childOffset.y=y;
}
void animation::setRect(std::string &name,float x,float y,float width,float height)
{
    if (!spriteExists(name)) return;
    sprites[name].spr->setSubRect(x,y,width,height);
}
void animation::setCenter(std::string &name,float x,float y)
{
    if (!spriteExists(name)) return;
    sprites[name].spr->setCenter(x,y);
}
void animation::setPosition(std::string &name,float x,float y)
{
    if (!spriteExists(name)) return;
    sprites[name].position.x=x;
    sprites[name].position.y=y;
}
void animation::setBlendmode(std::string &name,std::string &blend)
{
    if (!spriteExists(name)) return;
    blend=Lower(blend);
    if (blend=="none")
        sprites[name].spr->setBlendMode(noBlending);
    else if (blend=="add")
        sprites[name].spr->setBlendMode(additiveBlending);
    else if (blend=="multiply")
        sprites[name].spr->setBlendMode(multiplyBlending);
    else if (blend=="alpha")
        sprites[name].spr->setBlendMode(alphaBlending);
}
void animation::setScaleX(std::string &name,float x)
{
    if (!spriteExists(name)) return;
    sprites[name].scale.x=x;
}
void animation::setScaleY(std::string &name,float y)
{
    if (!spriteExists(name)) return;
    sprites[name].scale.y=y;
}
void animation::setAngle(std::string &name,float x)
{
    if (!spriteExists(name)) return;
    sprites[name].angle=x;
}
void animation::setTransparency(std::string &name,float x)
{
    if (!spriteExists(name)) return;
    sprites[name].transparency=x;
}
void animation::setVisible(std::string &name,bool x)
{
    if (!spriteExists(name)) return;
    sprites[name].spr->setVisible(x);
}
void animation::setColor(std::string &name,std::string &color,float x)
{
    if (!spriteExists(name)) return;
    //get rgb values
    int R1;
    int G1;
    int B1;
    int R2;
    int G2;
    int B2;
    sscanf (color.c_str(),"#%2x%2x%2x#%2x%2x%2x",&R1,&G1,&B1,&R2,&G2,&B2);
    //set
    sprites[name].spr->setColor(R1+x*(R2-R1),G1+x*(G2-G1),B1+x*(B2-B1));
}
void animation::move(std::string &name,std::string &path,float x)
{//set position according to path
    //check if path has more than 1 node
    if (!spriteExists(name)) return;
    if (paths[path].size()==1)
    {
        sprites[name].pathPos.x=paths[path][0].x;
        sprites[name].pathPos.y=paths[path][0].y;
        return;
    }
    else if (paths[path].size()<1)
        return;
    //get total distance of path
    float totalDist=getTotalDistance(path);
    if (totalDist==0) return; //danger of dividing by 0

    //x must be between 0 and 1
    if (x>=0 && x<1)
    {
        posVectorFloat pv=getPosition(path,x*totalDist);
        //setPosition(name,pv.x,pv.y);
        sprites[name].pathPos.x=pv.x;
        sprites[name].pathPos.y=pv.y;
    }
    else if (x<0)
    {//extrapolate from start
        posVectorFloat pv=getPositionExtra(path,x,totalDist,true);
        //setPosition(name,pv.x,pv.y);
        sprites[name].pathPos.x=pv.x;
        sprites[name].pathPos.y=pv.y;
    }
    else if (x>=1)
    {//extrapolate from end
        posVectorFloat pv=getPositionExtra(path,x-1,totalDist,false);
        //setPosition(name,pv.x,pv.y);
        sprites[name].pathPos.x=pv.x;
        sprites[name].pathPos.y=pv.y;
    }
}
void animation::playSound(std::string &name)
{
    if (gamedata && gamedata->playSounds)
        sounds[name]->Play(gamedata);
}

bool animation::spriteExists(std::string &name)
{
    return (sprites.find(name)==sprites.end())? false : true;
}
void animation::clearSprites()
{
    //delete all sprite objects
    std::map<std::string,animationSprite>::iterator it;
    for (it=sprites.begin();it!= sprites.end();it++)
    {
        delete (*it).second.spr;
    }
    //clear map
    sprites.clear();
    //and draw list
    drawSprites.clear();
}
void animation::clearSounds()
{
    //delete all sound objects
    std::map<std::string,sound*>::iterator it;
    for (it=sounds.begin();it!= sounds.end();it++)
    {
        delete (*it).second;
    }
    //clear map
    sounds.clear();
}


void animation::updateSprites()
{//updates all properties of the sprite
    std::map<std::string,animationSprite>::iterator it;
    for (it=sprites.begin();it!= sprites.end();it++)
    {
        animationSprite &animSprite=(*it).second;
        //do not update if it's a child
        if (animSprite.parent!="")
            continue;
        animSprite.spr->setPosition((animSprite.position.x+animSprite.pathPos.x)*globalScale+offset.x,
                                      (animSprite.position.y+animSprite.pathPos.y)*globalScale+offset.y);
        animSprite.spr->setScaleX(animSprite.scale.x*globalScale);
        animSprite.spr->setScaleY(animSprite.scale.y*globalScale);
        animSprite.spr->setRotation(animSprite.angle);
        animSprite.spr->setTransparency(animSprite.transparency);
        //update children
        updateChildren((*it).first);
    }
}
void animation::updateChildren(std::string parent)
{
    animationSprite &parentSprite=sprites[parent];
    for(size_t i=0;i<parentSprite.children.size();i++)
    {
        std::string &child=parentSprite.children[i];
        animationSprite &childSprite=sprites[child];
        //scale
        float psx=parentSprite.spr->getScaleX();
        float psy=parentSprite.spr->getScaleY();
        float csx=childSprite.scale.x;
        float csy=childSprite.scale.y;
        childSprite.spr->setScaleX(psx*csx);
        childSprite.spr->setScaleY(psy*csy);
        //rotation
        float pr=parentSprite.spr->getAngle();
        float cr=childSprite.angle;
        childSprite.spr->setRotation(pr+cr);
        //transparency
        float pt=parentSprite.spr->getTransparency();
        float ct=childSprite.transparency;
        childSprite.spr->setTransparency(pt*ct);
        //position
        float px=(parentSprite.spr->getX()-offset.x)/globalScale;
        float py=(parentSprite.spr->getY()-offset.y)/globalScale;
        float cx=childSprite.position.x+childSprite.pathPos.x+childSprite.childOffset.x;
        float cy=childSprite.position.y+childSprite.pathPos.y+childSprite.childOffset.y;
        float r=sqrt(cx*cx*psx*psx+cy*cy*psy*psy);
        float a=atan2(-cy*psy,cx*psx)*180/PI;
        childSprite.spr->setPosition(offset.x+(px+r*cos((pr+a)*PI/180))*globalScale,
                                        offset.y+(py+r*sin((pr+a)*PI/-180))*globalScale);
        //update children
        updateChildren(child);
    }
}
void animation::resetSprites()
{
    std::map<std::string,animationSprite>::iterator it;
    for (it=sprites.begin();it!= sprites.end();it++)
    {
        (*it).second.pathPos.x=0;
        (*it).second.pathPos.y=0;
        (*it).second.position.x=0;
        (*it).second.position.y=0;
        (*it).second.scale.x=1;
        (*it).second.scale.y=1;
        (*it).second.angle=0;
        (*it).second.transparency=0;
        if ((*it).second.parent!="")
            (*it).second.transparency=1;
    }
}
double animation::getLocalTimer(std::string type,double s,double e,double t,double alpha,double beta)
{//input t must go from 0 to 1
    double out=0;
    if(type=="linear")
        out=(e-s)*t+s;
    else if(type=="quadratic")
        out=(e-s)*t*t+s;
    else if(type=="squareroot")
        out=(e-s)*pow(t,0.5)+s;
    else if(type=="cubic")
        out=(e-s)*t*t*t+s;
    else if(type=="cuberoot")
        out=(e-s)*pow(t,1.0/3.0)+s;
    else if(type=="exponential")
        out=(e-s)/(exp(alpha)-1)*exp(alpha*t)+s-(e-s)/(exp(alpha)-1);
    else if(type=="elastic") //beta=wavenumber
        out=(e-s)/(exp(alpha)-1)*cos(beta*t*2*PI)*exp(alpha*t)+s-(e-s)/(exp(alpha)-1);
    else if(type=="sin")
        out=s+e*sin(alpha*t*2*PI); //s=offset, e=amplitude, alpha=wavenumber
    else if(type=="cos")
        out=s+e*cos(alpha*t*2*PI); //s=offset, e=amplitude, alpha=wavenumber
    return out;
}
float animation::getTotalDistance(std::string &path)
{
    if (paths[path].size()<=1) return 0;
    float distance=0;
    //loop through nodes
    std::vector<posVectorFloat>::iterator it;
    posVectorFloat previous(0,0);
    for ( it=paths[path].begin()+1 ; it < paths[path].end(); it++ )
    {
        posVectorFloat diff=*(it-1)-*(it);
        distance+=sqrt(diff.x*diff.x+diff.y*diff.y);
    }
    return distance;
}
posVectorFloat animation::getPosition(std::string &path,float target)
{
    float distance=0;
    float nodeDistance=0;
    //loop through nodes
    std::vector<posVectorFloat>::iterator it;
    posVectorFloat previous(0,0);
    for ( it=paths[path].begin()+1 ; it < paths[path].end(); it++ )
    {
        posVectorFloat diff=*(it-1)-*(it);
        nodeDistance=sqrt(diff.x*diff.x+diff.y*diff.y);
        distance+=nodeDistance;
        //distance reached: interpolate
        if (distance>target)
        {
            float overshoot=distance-target;
            float ratio=overshoot/nodeDistance;
            posVectorFloat out(0,0);
            out.x=(*it).x-ratio*((*it).x-(*(it-1)).x);
            out.y=(*it).y-ratio*((*it).y-(*(it-1)).y);
            return out;
        }
    }

    return posVectorFloat();
}
posVectorFloat animation::getPositionExtra(std::string &path,float target,float totalDist,bool start)
{
    posVectorFloat diff;
    std::vector<posVectorFloat>::iterator it;
    int x=0;
    if (start)
    {//get first nodes
        it=paths[path].begin()+1;
        diff=*(it-1)-*(it);
        x=-1;
    }
    else
    {//get end nodes
        it=paths[path].end()-1;
        diff=*(it-1)-*(it);
    }
    //get ratio of difference
    float nodeDistance=sqrt(diff.x*diff.x+diff.y*diff.y);
    float ratio=nodeDistance/totalDist;
    //get direction
    posVectorFloat out(0,0);
    out.x=(*(it+x)).x-target/ratio*(diff.x);
    out.y=(*(it+x)).y-target/ratio*(diff.y);
    return out;
}

}
