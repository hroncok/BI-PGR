Název fakulty:		Fakulta informačních technologií
Název předmětu:		Počítačová grafika
Zkratka předmětu:	BI-PGR
Rok:				2012
Měsíc:				prosinec
Den:				úterý
Jméno:				Miroslav
Příjmení:			Hrončok
Uživatelské jméno:	hroncmir
Jméno cvičícího:	Tomáš
Příjmneí cvičícího:	Barák

==== Zadání ==== 
Rád bych vytvořil scénu výrobní linky na lahvové pivo: lahve jedou na pásech a je do nich vléváno pivo, lahve jsou uzavírány zátkovačem a nacpány do bas, které odváží kamion. Celá scéna je v hale. Uživatel může linku vypínat a zapínat, měnit barvu piva a lahví.

Při výběru tématu semestrální práce jsem netušil, kolik to zabere práce, proto přiznávám, že jsem zadání nesplnil. Držel jsem se spíše jednotlivých bodů, které bylo potřeba splnit.

---- 


==== Ovládání ==== 
Scénu lze ovlivňovat za běhu pomocí kontextové nabídky (pravé tlačítko myši někam do prostoru scény). V nabídce lze přepnout kameru (dvě statické a jedna volná, která začne na současné pozici), vypnout/zapnout reflektor a/nebo animaci, případně vypsat do terminálu vývpjové informace nebo aplikaci vypnout.

Všechny položky nabídky mají u sebe uvedenou klávesovou zkratku.

Při zapnuté volné kameře je pohyb po scéně realizovaný pomocí kurozorových šipek a kláves Page Up/Down.

---- 

==== Konfigurace ==== 
Součástí aplikace je konfigurační soubor, který umožňuje měnit body a vektory, které ovlivňují trasu a počet lahví.

První číslo udává počet lahví, druhé počet fragmentů křivky, po které jezdí.

Následuje vždy dvojice souřadnic bodu křivky a dvojice souřadnic výstupního vektoru v daném bodu.

Jednotlivé položky jsou odděleny libovolným množstvím whitespacu a nejsou kontrolovány na smysluplnost, můžete tak například na scénu dát tolik lahví, že se navzájem kříží.

---- 

==== Soubory ==== 
Soubory ve složce resources pocházejí z cvičení sceneGraph a nejsem jejich autorem (kromě MeshNode.vert a .frag, které jsem značně upravil).

Ve složce data jsou modely a textury načítané za běhu programu. Jsem jejich autorem, kromě podlahy, kterou jsem našel na Google Images.

Soubory v rootu jsou mé (AnimMode vychází z podobného souboru z cvičení sceneGraph, stejně jako main).

---- 

