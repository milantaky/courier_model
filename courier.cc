#include "simlib.h"

int balicky = 0;            // počítač balíčků

Store kapacita_auta("Prostor", 80);
Store sklad("Polozky", 9999);       // asi pak napsat nejaky mega cislo proste
Facility Kuryr("kuryr");
Facility Rozvoz("Je co rozvazet");
Stat dobaVSystemu("Celkova doba rozvozu");


class Rozvazeni : public Process {
public:
    void Behavior() {
    Store balicky_v_aute("Balicky", balicky);
        //printf("%d", balicky_v_aute.Capacity());

        Seize(Kuryr);
        double tvstup = Time;
        int nevyzvednute = 0;
        Wait(300);          // Priprava na rozvazeni
        while(!balicky_v_aute.Full())       // Dokud je co -> rozvazej
        {
            /*if((Time - tvstup)/3600 > 4.0 && (Time - tvstup)/3600 < 5.0)
                Wait(3600);*/
            Seize(Rozvoz);

            Enter(balicky_v_aute, 1);
            Wait(Exponential(400));         // Dojezd na misto

            if(Random()<= 0.90)
            {
                // Uspesne doruceni
                Wait(Exponential(60));
                Release(Rozvoz);
            }
            else
            {
                // Neni doma
                nevyzvednute++;
                Release(Rozvoz);
            }
        }
        //printf(" ktere rozvezl za: %.1fh rozvezl a vrací se s: %d\n\n", (Time - tvstup)/60/60, nevyzvednute);
        dobaVSystemu((Time - tvstup)/60/60);
        Leave(sklad, nevyzvednute);
        Leave(kapacita_auta, kapacita_auta.Capacity());
        Release(Kuryr);
    }
};


class Nakladka : public Process {
public:
    void Behavior() {
        Seize(Kuryr);
        double tvstup_nak = Time;
        balicky = 0; 
        while(!kapacita_auta.Full())
        {
            if(Random()<= 0.80)
            {
                // Maly balík
                Enter(sklad, 1);
                Enter(kapacita_auta, 1);
                Wait(Exponential(30));
            }
            else
            {
                // Velký balík
                Enter(kapacita_auta, 1);    // nejdriv vezmu jedno místo a podívam se jestli je ještě volno
                if(kapacita_auta.Full())
                {
                    // Bylo místo jen na jeden balík, no nic odjíždím
                    Leave(kapacita_auta, 1); 
                    continue;
                }
                else
                {
                    // Místo na dva
                    Enter(sklad, 1);
                    Enter(kapacita_auta, 1);
                    Wait(Exponential(60));
                }
            }
            balicky++;  // balíček ve voze
        }
        // Konec nakládání
        Release(Kuryr);
        //printf("Za: %.1fmin nalozil: %d balicku", (Time - tvstup_nak)/60, balicky);
        (new Rozvazeni)->Activate();
        (new Nakladka)->Activate();
    }
};

int main()
{
    Init(0, 999999999);            //random cislo
    (new Nakladka)->Activate();
    Run();
    dobaVSystemu.Output();
}