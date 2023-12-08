#include "simlib.h"

Store kapacita_auta("Prostor", 80);
Store sklad("Polozky", 9999);       // asi pak napsat nejaky mega cislo proste
Facility Kuryr("kuryr");

int balicky = 0;            // počítač balíčků

class Nakladka : public Process {
public:
    void Behavior() {
        double tvstup = Time;
        Seize(Kuryr);
        while(!kapacita_auta.Full())
        {
            if(Random()<= 0.80)
            {
                // Maly balík
                Enter(sklad, 1);
                Enter(kapacita_auta, 1);
                Wait(30);
            }
            else
            {
                // Velký balík
                Enter(kapacita_auta, 1);    // nejdriv vezmu jedno místo a podívam se jestli je ještě volno
                if(kapacita_auta.Full())
                {
                    // Bylo místo jen na jeden balík, no nic odjíždím
                    balicky--;
                    break;
                }
                else
                {
                    // Místo na dva
                    Enter(sklad, 1);
                    Enter(kapacita_auta, 1);
                    Wait(60);
                }
            }
            balicky++;  // balíček ve voze
        }
        // Konec nakládání
        Release(Kuryr);
        Leave(kapacita_auta, 80);
        printf("balicky: %d\n", balicky);
        balicky = 0; 
    }
};

// Tady nevím, jak to jinak generovat, takže zatím takhle
class Generator : public Event {
    void Behavior() {
	(new Nakladka)->Activate();
		Activate(Time+10);      // + 10 je tady jen tak, nevím co tam dat
    }
};


int main()
{
    Init(0, 100000);
    (new Generator)->Activate();
    Run();
    
}
