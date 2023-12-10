/**************************************************************/
/*         Projekt do předmětu modelování a simulace          */
/*  Zadaní číslo 5:                                           */
/*  SHO Model logistiky - rozvoz/transport zboží a materiálu  */
/*                                                            */
/*  Autoři: Jakub Valeš, xvales04                             */
/*          Milan Takáč, xtakac09                             */
/*                                                            */
/*                                          10. prosince 2024 */
/**************************************************************/

#include "simlib.h"

int balicky = 0;            // počítač balíčků
double tvstup_nak = 0;      // doba nakládání zásilek

Store kapacita_auta("Prostor", 97);         // Prostor pro zásilky v dodávce
Store sklad("Polozky", 9999);               // Položky připravené na skladu

Facility Kuryr("kuryr");                    // Obslužná linka kurýr
Facility Rozvoz("Je co rozvazet");          // Obslužná linka pro rozvoz

// Statistiky
Stat dobaVSystemu("Pracovní doba kurýra");
Stat najeteKM("Najete kilometry za den");
Stat casJizdy("Celkový čas jízdy kurýra");
Stat pocet_bal("Počet naložených zásilek");
Stat nalozeni("Doba naložení zásilek");
Stat jeden_rozvoz("Doba přepravy jedné zásilky");

// Proces rozvážení zásilek
class Rozvazeni : public Process {
public:
    void Behavior() {
    Store balicky_v_aute("Balicky", balicky);

        Seize(Kuryr);
        double tnalozeni = (Time - tvstup_nak);         // cas nalozeni
        double tvstup = Time;                           // cas rozvazeni i s pauzou a dorucovanim
        double doruceni;                                
        double cel_doruceni = 0;                        // celkový čas, který stráví předání zásilky zákazníkovi 

        int nevyzvednute = 0;
        Wait(300);                                      // Priprava na rozvazeni
        while(!balicky_v_aute.Full())                   // Dokud je co -> rozvazej
        {

            if((Time - tvstup)/3600 > 4.5 && (Time - tvstup)/3600 < 5.0)        // Po 4.5h má kurýr právo si udělat pauzu na 30min
                Wait(1800);
            
            Seize(Rozvoz);
            Enter(balicky_v_aute, 1);

            double jizda = Normal(6*60, 60);
            Wait(jizda);                                                        // Dojezd na misto
            jeden_rozvoz(jizda/60);
            

            if(Random()<= 0.90)
            {
                // Uspesne doruceni
                doruceni = Exponential(60);
                Wait(doruceni);
                cel_doruceni += doruceni;

                Release(Rozvoz);
            }
            else
            {
                // Nepodařilo se zastohnout zákazníka
                nevyzvednute++;
                Release(Rozvoz);
            }
        }

        double navrat = Uniform(10*60, 15*60);
        Wait(navrat);                                                     // Kurýr se vrací na sklad
        
        casJizdy((Time - tvstup - 1800 - cel_doruceni)/60/60);
        dobaVSystemu((Time - tvstup + tnalozeni)/60/60);
        najeteKM(((Time - tvstup - 1800 - cel_doruceni)/60/60) * 30);

        Leave(sklad, nevyzvednute);                                      // Navrácení nevyzvednutých zásilek na sklad
        Leave(kapacita_auta, kapacita_auta.Capacity());                  // Obnovení kapacity dodávky pro nové zásilky
        Release(Kuryr);
    }
};

// Proces nakládání zásilek do dodávky
class Nakladka : public Process {
public:
    void Behavior() {
        Seize(Kuryr);
        tvstup_nak = Time;                                              // Čas nakládání zásilek
        balicky = 0;                                                    // Počet naložených zásilek
        // Dokuď není auto plné -> nakládej zásilky
        while(!kapacita_auta.Full())
        {
            float pts = Random();
            if(pts <= 0.60)
            {
                // Maly balík
                Enter(sklad, 1);
                Enter(kapacita_auta, 1);
                Wait(30);
            }
            else if(pts > 0.60 && pts <= 0.9)
            {
                // Střední balík
                Enter(kapacita_auta, 1);    // nejdriv vezmu jedno místo a podívam se jestli je ještě volno
                if(kapacita_auta.Full())
                {
                    Leave(kapacita_auta, 1); 
                    continue;
                }
                else
                {
                    // Místo na dva
                    Enter(sklad, 1);
                    Enter(kapacita_auta, 1);
                    Wait(45);
                }
            }
            else
            {
                //Velký balík
                Enter(kapacita_auta, 1);    // nejdriv vezmu jedno místo a podívam se jestli je ještě volno
                if(kapacita_auta.Full())
                {
                    Leave(kapacita_auta, 1); 
                    continue;
                }

                Enter(kapacita_auta, 1);    // nejdriv vezmu jedno místo a podívam se jestli je ještě volno
                if(kapacita_auta.Full())
                {
                    Leave(kapacita_auta, 1); 
                    continue;
                }
                else
                {
                    // Místo na tři
                    Enter(sklad, 1);
                    Enter(kapacita_auta, 1);
                    Wait(60);
                }
            }
            balicky++;                                      // Zásilky naložené ve voze
        }
        nalozeni((Time - tvstup_nak)/60);
        
        // Konec nakládání
        pocet_bal(balicky);
        Release(Kuryr);
        (new Rozvazeni)->Activate();
        (new Nakladka)->Activate();
    }
};

int main()
{
    Init(0, 9999999);               // Čas simulace
    (new Nakladka)->Activate();
    Run();

    // Výpis statistik
    //pocet_bal.Output();
    //jeden_rozvoz.Output();
    dobaVSystemu.Output();
    casJizdy.Output();
    najeteKM.Output();
    nalozeni.Output();
}