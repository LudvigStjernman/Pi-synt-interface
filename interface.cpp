#include <iostream> //I/O för debugging
#include <string>
#include <chrono>
#include <algorithm>
#include <SFML/Graphics.hpp> //Library för grafik
#include "socket.h"
#include "font.h" //data för typsnittet för texten
#include "midi.h" //klass för att hantera midi

struct Slider //klass för slider
{
    sf::RectangleShape shp; //en fyrkant att representera slidern
    sf::Shader shdr; //shader till fyrkanten (för att ändra utseendet på fyrkanten)
    float Value = 0; //sliderns värde mellan 0 och 1
    int headsz = 0; //storlek för den delen av slidern som flyttas
    sf::Vector3f primcol; //primär färg
    sf::Vector3f seccol; //bakgrundsfärg
    sf::Vector2i pos; //position för det över vänstra hurnet av slidern
    sf::Vector2f size; //storlek i x och y-led
    static int sc, cs; //sc är antalet sliders på skärmen, cs är den slidern som förflyttas. static betyder att värdena delas av alla sliders
    int m_sc = 0; //sliderns "id"
    bool horizontal = true, ran = false, locked = false; //horizontal bestämmer på vilket led slidern ska röra sig, ran håller koll på om "draw"-funktionen redan har körts eller inte, locked bestämmer om slidern används sen tidigare
    void Draw(sf::RenderWindow &wnd) //rita ut slidern på skärmen
    {
        sf::Vector2f uval(0, 0); //variabel för att hålla koll på musens position
        if (!ran) //om detta är den första gången slidern ritas up
        {
            ran = true;
            if(horizontal) //om slidern är horisontell
                shdr.setUniform("mpos", (sf::Vector2f)pos); //sätt slidern till vänster
            else
                shdr.setUniform("mpos", (sf::Vector2f)pos + sf::Vector2f(0, size.y)); //sätt slidern till botten
        }
        if (!cs || cs == m_sc) //om denna är slidern som används just nu, eller ingen annan används
            if (shp.getGlobalBounds().contains((sf::Vector2f)sf::Mouse::getPosition(wnd)) && sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) || locked) //om musens position ligger på slidern eller slidern används sen tidigare
            {
                cs = m_sc; //sätt att denna slider är vald
                if (shp.getGlobalBounds().contains( //om musens x eller y värde ligger inom slidern
                        horizontal ? sf::Mouse::getPosition(wnd).x : (pos.x + 1),
                        horizontal ? (pos.y + 1) : sf::Mouse::getPosition(wnd).y))
                {
                    if(horizontal){ //om slidern är horisontell
                        Value = std::clamp((float)sf::Mouse::getPosition(wnd).x - pos.x, 0.f, size.x) / size.x; //sätt sliderns värde relativt till musens x-värde
                    } else{
                        Value = 1.f - std::clamp((float)sf::Mouse::getPosition(wnd).y - pos.y, 0.f, size.y) / size.y; //sätt sliderns värde relativt till musesn y-värde
                    }
                }
                locked = true;
            }
            uval = sf::Vector2f(Value * size.x + pos.x, pos.y + size.y * (1.f - Value)); //beräkna sliderns position utifrån värdet
            shdr.setUniform("mpos", uval); //sätt sliderns position
        if (!sf::Mouse::isButtonPressed(sf::Mouse::Button::Left)) //om inte vänsterknappen är nedtryckt
        {
            locked = false; //slidern används inte
            if (cs == m_sc) //om den aktiva slidern är denna
                cs = 0; //ingen slider används
        }
        wnd.draw(shp, &shdr); //rita slidern på skärmen med shadern
    }
    Slider(int w, int h, int x, int y, sf::Vector3f prmcol, sf::Vector3f scdcol) //konstruktör för slidern
    {
        m_sc = ++sc;
        horizontal = w > h;
        primcol = prmcol;
        seccol = scdcol;
        size.x = w;
        size.y = h;
        pos.x = x;
        pos.y = y;
        headsz = w > h ? h : w;
        shp.setSize(size);
        shp.setPosition(x, y);
        std::string strpclr = std::to_string(primcol.x) + "f, " + std::to_string(primcol.y) + "f, " + std::to_string(primcol.z) + "f",
                    strsclr = std::to_string(seccol.x) + "f, " + std::to_string(seccol.y) + "f, " + std::to_string(seccol.z) + "f",
                    strhsz = std::to_string(headsz) + ".0"; //gör om färgerna till strings

        std::string shdrcde = "#version 120\n" //kod för shadern som körs på grafikkortet
                              "uniform vec2 mpos;\n"
                              "void main(){\n"
                              "vec3 pcol = vec3(" +
                              strpclr + ") / 255.f;\n"
                                        "vec3 scol = vec3(" +
                              strsclr + ") / 255.f;\n"
                                        "vec2 ifc = (vec2(1, -1) * gl_FragCoord.xy + vec2(0, 480));\n"
                                        "vec3 clr = mix(pcol, scol, clamp(distance(ifc." +
                              (horizontal ? "x" : "y") + ", mpos." + (horizontal ? "x" : "y") + "), 0.f, " + strhsz + ") / " + strhsz + ");\n"
                                                                                                                                        "gl_FragColor = vec4(clr, 1.f);\n"
                                                                                                                                        "}";
        if (!shdr.loadFromMemory(shdrcde, sf::Shader::Fragment)) //ladda shadern
            std::cout << "Fragment shader error";
    }
};

struct Envelope //klass för envelope
{
    sf::Vertex vs[5]; //ändpunkterna för linjerna som ritas ut
    float *a = 0, *d = 0, *s = 0, *r = 0; //pekare till värdena för attack, decay, sustain och release
    float w = 0, h = 0; //bredd och höjd
    sf::Vector2i pos; //position
    static bool noteDown; //om en not är nedtryckt
    bool internal_notedown = false; //om en not är registrerad
    int amplitude = 0; //amplitud
    void BindParams(float *_a, float *_d, float *_s, float *_r) //sätt alla parametrar till addresser
    {
        a = _a;
        d = _d;
        s = _s;
        r = _r;
    }
    void Draw(sf::RenderWindow &wnd) //rita ut på skärmen
    {
        vs[1].position.x = pos.x + w * (*a) / 4; //beräkna positionen för alla linjer
        vs[2].position = sf::Vector2f(vs[1].position.x + (*d) * w / 4, pos.y + h - (*s) * h);
        vs[3].position = sf::Vector2f(vs[2].position.x + w / 4, vs[2].position.y);
        vs[4].position = sf::Vector2f(vs[3].position.x + w * (*r) / 4, pos.y + h);
        wnd.draw(vs, 5, sf::LineStrip); //rita ut
    }
    Envelope(int _w, int _h, int x, int y) //konstruktör
    {
        pos = sf::Vector2i(x, y);
        w = _w;
        h = _h;
        vs[0] = sf::Vertex((sf::Vector2f)pos + sf::Vector2f(0, h));
        vs[1] = sf::Vertex((sf::Vector2f)pos + sf::Vector2f(w / 10, 0));      // x = a
        vs[2] = sf::Vertex((sf::Vector2f)pos + sf::Vector2f(w / 5, h));       // x = d, y = s
        vs[3] = sf::Vertex((sf::Vector2f)pos + sf::Vector2f(3 * w / 4, h));   // x = const, y = s
        vs[4] = sf::Vertex((sf::Vector2f)pos + sf::Vector2f(17 * w / 20, h)); // x = r
        vs[0].color = sf::Color(11, 255, 255);
        vs[1].color = sf::Color(11, 255, 255);
        vs[2].color = sf::Color(11, 255, 255);
        vs[3].color = sf::Color(11, 255, 255);
        vs[4].color = sf::Color(11, 255, 255);
    }
};

bool Envelope::noteDown = false;

int Slider::sc = 0;
int Slider::cs = 0;

struct Button{
    enum Usage{
        press,
        toggle
    };
    sf::RectangleShape shp;
    sf::Text btntxt;
    std::vector<Button*> latchBtns;
    template <class... Buttons>
    void BindLatch(Buttons&&...  latch){
        (latchBtns.push_back(&latch), ...);
        for(auto r : latchBtns){
            if(std::find(r->latchBtns.begin(), r->latchBtns.end(), this) != r->latchBtns.end())
                r->latchBtns.push_back(this);
        }
    }
    void draw(sf::RenderWindow &wnd){
        if(fRen = !fRen){
            wnd.draw(shp);
            wnd.draw(btntxt);
        }

        if(shp.getGlobalBounds().contains((sf::Vector2f)sf::Mouse::getPosition(wnd))){
            shp.setOutlineThickness(2);
            if(sf::Mouse::isButtonPressed(sf::Mouse::Button::Left) && !mprs){
                
            }
        } else shp.setOutlineThickness(0);
    }
    Button(std::string text, Usage state, sf::Color primCol, sf::Color secCol, int x, int y, int w, int h){
        shp.setOutlineColor(prcol);
    }
    bool getIfPressed(){
        if(_state == Usage::press) return mprs && !(mprs = false);
        if(_state == Usage::toggle) return mprs;
    }
    private:
    Usage _state;
    sf::Color prcol, sccol, trclr;
    bool mprs = false;
    bool fRen = false;
};

int main()
{
    Slider::sc = 0;
    Slider::cs = 0;
    sf::ContextSettings cs; //fönsterkontext (gör att man kan ha fler inställningar till fönstret)
    cs.antialiasingLevel = 8; //sätt antialiasing-nivån till 8
    sf::RenderWindow window(sf::VideoMode(800, 480), "Synt"/*, sf::Style::Fullscreen, cs*/); //skapa fönstret med matchande storlek till fönstret
    sf::RectangleShape Env1s(sf::Vector2f(225, 125)), //skapa rektanglar med bestämda storlekar som bakgrunder till olika funktioner
        Env2s(sf::Vector2f(225, 125)),
        Vcfs(sf::Vector2f(225, 125)),
        Vcas(sf::Vector2f(50, 405)),
        Lfos(sf::Vector2f(225, 125)),
        olS1(sf::Vector2f(300, 239)), //de som börjar med ol är rektanglar med samma färg som bakgrunden och som har en ytterlinje (de blåa linjerna på skärmen)
        olS2(sf::Vector2f(300, 239)),
        olS3(sf::Vector2f(300, 239)),
        olS4(sf::Vector2f(300, 239));

    sf::Vector3f pcol(11, 255, 255), scol(20, 20, 20); //sätt primärfärg (ljusblå) och bakgrundsfärg (grå)

    Slider Env1sda(225, 20, 550, 155, pcol, scol), //sliders till alla parametrar
        Env1sdd(225, 20, 550, 180, pcol, scol),
        Env1sdr(225, 20, 550, 205, pcol, scol),
        Env1sds(20, 125, 525, 25, pcol, scol),
        Env2sda(225, 20, 550, 255, pcol, scol),
        Env2sdd(225, 20, 550, 280, pcol, scol),
        Env2sdr(225, 20, 550, 305, pcol, scol),
        Env2sds(20, 125, 525, 330, pcol, scol),
        Vcasd(20, 405, 430, 25, pcol, scol),
        Lfosld(225, 20, 25, 305, pcol, scol);

    Envelope Env1(227, 123, 551, 26), //envelopes
        Env2(227, 123, 551, 331);

    olS1.setFillColor(sf::Color(50, 50, 50)); //sätt ut allt med färger och positioner
    olS1.setPosition(0, 0);

    olS2.setFillColor(sf::Color(50, 50, 50));
    olS2.setPosition(500, 0);

    olS3.setFillColor(sf::Color(50, 50, 50));
    olS3.setPosition(500, 241);

    olS4.setFillColor(sf::Color(50, 50, 50));
    olS4.setPosition(0, 241);

    olS1.setOutlineColor(sf::Color(11, 255, 255)); //sätt ytterlinjernas storlekar och färger
    olS1.setOutlineThickness(2);

    olS2.setOutlineColor(sf::Color(11, 255, 255));
    olS2.setOutlineThickness(2);

    olS3.setOutlineColor(sf::Color(11, 255, 255));
    olS3.setOutlineThickness(2);

    olS4.setOutlineColor(sf::Color(11, 255, 255));
    olS4.setOutlineThickness(2);

    Env1s.setFillColor(sf::Color::Black);
    Env1s.setPosition(550, 25);

    Env2s.setFillColor(sf::Color::Black);
    Env2s.setPosition(550, 330);

    Vcfs.setFillColor(sf::Color::Black);
    Vcfs.setPosition(25, 25);

    Vcas.setFillColor(sf::Color::Black);
    Vcas.setPosition(375, 25);

    Lfos.setFillColor(sf::Color::Black); //fyrkanten för lfon
    Lfos.setPosition(25, 330);

    sf::Font font;

    if (!font.loadFromMemory(nulshock_bd_otf, nulshock_bd_otf_len)) //försök läsa in typsnittet
    {
        printf("Kunde inte läsa in typsnitt\n");
    }

    sf::Text vcat, lfot, vcft, env1t, env2t, E1a, E1d, E1r, E1s, E2a, E2d, E2r, E2s, lfofrt; //all text som används

    vcat.setFont(font); //förbered text
    vcat.setString("VCA");
    vcat.setCharacterSize(17);
    vcat.setPosition(377, 430);
    vcat.setFillColor(sf::Color(11, 255, 255));
    lfot = vcat;
    lfot.setString("LFO");
    lfot.setPosition(25, 455);
    vcft = vcat;
    vcft.setString("VCF");
    vcft.setPosition(25, 0);
    env1t = vcat;
    env1t.setString("ENV1");
    env1t.setPosition(550, 0);
    env2t = vcat;
    env2t.setString("ENV2");
    env2t.setPosition(550, 455);

    E1a = vcat;
    E1a.setCharacterSize(15);
    E1a.setPosition(528, 155);
    E1a.setString("A");

    E1d = E1a;
    E1d.setString("D");
    E1d.setPosition(528, 180);

    E1r = E1a;
    E1r.setString("R");
    E1r.setPosition(528, 205);

    E1s = E1a;
    E1s.setString("S");
    E1s.setPosition(505, 25);

    E2a = E1a;
    E2a.move(0, 100.f);

    E2d = E1d;
    E2d.move(0, 100.f);

    E2r = E1r;
    E2r.move(0, 100.f);

    E2s = E1s;
    E2s.setPosition(505, 330);

    lfofrt = E2d;
    lfofrt.setString("Frekvens");
    lfofrt.move(-504, 0);

    Env1.BindParams(&Env1sda.Value, &Env1sdd.Value, &Env1sds.Value, &Env1sdr.Value); //bind parametrar för envelopes till värdena för rätt sliders
    Env2.BindParams(&Env2sda.Value, &Env2sdd.Value, &Env2sds.Value, &Env2sdr.Value);

    int lfostate = 0;
    window.clear(sf::Color(50, 50, 50)); //rita över allting med bakgrundsfärgen
    window.draw(olS1);
    window.draw(olS2);
    window.draw(olS3);
    window.draw(olS4);
    window.draw(Env1s);
    window.draw(Env2s);
    window.draw(Vcfs);
    window.draw(Vcas);
    window.draw(vcat);
    window.draw(lfot);
    window.draw(vcft);
    window.draw(env1t);
    window.draw(env2t);
    window.draw(E1a);
    window.draw(E1d);
    window.draw(E1r);
    window.draw(E1s);
    window.draw(E2a);
    window.draw(E2d);
    window.draw(E2r);
    window.draw(E2s);
    window.draw(lfofrt);

    sf::Shader Lfoshdr; //shader för att visa vågformen på LFOn
    Lfoshdr.loadFromMemory(
        "#version 120\n"\
        "uniform int state;"\
        "uniform float freq;\n"\
        "void main(){\n"\
        "gl_FragColor = vec4(0, 0, 0, 1);\n"\
        "vec2 px = gl_FragCoord.xy - vec2(25, 24);"\
        "if(state == 0)\n"\
        "gl_FragColor += vec4(11/255, 1, 1, 1) * int(abs((mod(int(floor(px.x/(90*freq))), 2)) * 127 - px.y) <= 5 || mod(int(px.x), 90*freq) <= 4);\n"\
        "else if (state == 1)\n"\
        "gl_FragColor += vec4(11/255, 1, 1, 1) * int(abs(abs(mod(px.x,freq*90) - freq*90/2)*2/(freq*90) - (px.y-10)*1.2/125.f) <= 0.1f);\n"\
        "else if(state == 2)\n"\
        "gl_FragColor += vec4(11/255, 1, 1, 1) * int(abs(59.5*sin(px.x/(90*freq)) - (px.y - 63)) <= 4);\n"\
        "}"
        ,
        sf::Shader::Fragment);

    while (window.isOpen()) //så länge fönstret är öppet
    {
        sf::Event event;
        while (window.pollEvent(event)) //kolla om fönstret har stängts
        {
            if (event.type == sf::Event::Closed)
                window.close();
        }
if(sf::Keyboard::isKeyPressed(sf::Keyboard::Key::Escape))
window.close();

        Lfoshdr.setUniform("state", 2); //sätt vågform och frekvens till LFO-shadern
        Lfoshdr.setUniform("freq", Lfosld.Value + 0.1f);

        //rita ut objekt på skärmen
        
        window.draw(Env1s);
        window.draw(Env2s);
        window.draw(Lfos, &Lfoshdr);

        Env1sda.Draw(window);
        Env1sdd.Draw(window);
        Env1sdr.Draw(window);
        Env1sds.Draw(window);

        Env2sda.Draw(window);
        Env2sdd.Draw(window);
        Env2sdr.Draw(window);
        Env2sds.Draw(window);

        Vcasd.Draw(window);

        Env1.Draw(window);
        Env2.Draw(window);

        Lfosld.Draw(window);

        window.display();
        
    }

    return 0;
}
