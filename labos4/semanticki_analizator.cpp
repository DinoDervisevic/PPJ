#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <utility>
#include <fstream>
#include <algorithm>
#include <set>
#include <queue>
#include <climits> // Add this include for INT_MAX and INT_MIN
#include <sstream>
#include <stack>

using namespace std;

vector <string> kod;

vector<string>kod_za_main;

int registri = 5;
int vrhStoga = 40000;
int brojPusheva = 0;
bool retSeDesio = false;
int elseLabel = 0;
string trenutniArray = "";
int brojPushevaArray = 0;
bool neUzimajAdresu = false;
bool zaMain = false;

//
//
//MOGUCI PROBLEM: za brojPusheva resetiram na 0 kad se pozove funkcija, no postoji mogucnost da je to krivo, ne mogu sad smislit jel to oekj uvijek
//
//

class Node_svojstva{
    public:
        string znak = ""; //uniformni znak(tipa IDN, BROJ...)
        int redak = -1;
        string leks_jedinka = ""; //origigi sto pise (a, 12, +...)
        bool konst = false; //true je i ako je niz(const(tip))
        string tip = "";
        string ntip = ""; //nasljedni tip bitan za provjeru tocnosti deklaracija
        bool l_izraz = "";
        vector<string> argumenti = {}; //samo za funkcije
        vector<string> argumenti_imena = {}; //isto za funkcije
        int broj_elemenata = 0; //samo za deklaracije nizova
        vector<string> tipovi = {}; //samo za inicijalizaciju nizova (tj <inicijalizator>)
        bool fja_definirana = false; //gledamo ima li funckija definiciju ili samo deklaraciju
        bool jeParametar = false; //gledamo je li varijabla parametar funkcije

        Node_svojstva(string znak, int redak, string leks_jedinka) : znak(znak), redak(redak), leks_jedinka(leks_jedinka), konst(false) {}

        Node_svojstva() = default;

        Node_svojstva(string znak, int redak, string leks_jedinka, bool konst) : znak(znak), redak(redak), leks_jedinka(leks_jedinka), konst(konst) {}

};

class Tablica_Node;

class Node{
    public:
        int identifikator;
        Node_svojstva* svojstva = nullptr;
        Node* roditelj = nullptr;
        vector<Node*> djeca;
        Tablica_Node* tablica = nullptr;

        Node(Node_svojstva* svojstva) : svojstva(svojstva){};

        Node(){
            svojstva = new Node_svojstva();
        };

};

class Tablica_Node{
    public:
        map<string, Node*> zapis;
        Tablica_Node* roditelj = nullptr;
        vector<Tablica_Node*> djeca = {};

        map<string, int> adresa_na_stogu;
        int relativni_vrh = 0;
        bool main = false;

        vector<Node*> deklarirane_funkcije = {};
        vector<Node*> definirane_funkcije = {};

        Tablica_Node(Tablica_Node* roditelj = nullptr) : roditelj(roditelj) {}
};

map<string, Node*> funkcije;



//---------------------------
//#gl
bool isGlobal = true;
bool aktivnaDeklaracija = false;
Node* trenutniIzravniDeklarator;
//#ret
bool aktivnaNaredbaSkoka = false;
map <string, string> adresa; // tako da znam gdje je na stogu koja varijabla
//#vb
int brojacVelikihBrojeva = 0;
//---------------------------



vector<string> split(const string& str, const string& delimiter) {
    vector<string> dijelovi;
    size_t pocetak = 0;
    size_t kraj = str.find(delimiter);
    while (kraj != string::npos) {
        dijelovi.push_back(str.substr(pocetak, kraj - pocetak));
        pocetak = kraj + delimiter.length();
        kraj = str.find(delimiter, pocetak);
    }
    dijelovi.push_back(str.substr(pocetak));
    return dijelovi;
}


void greska(Node* node){ //napraviti funkciju za ispis greske
    cout << node->svojstva->znak << " ::=";
    for(Node* dijete : node->djeca){
        cout << " ";
        if(dijete->svojstva->redak != -1)
        cout << dijete->svojstva->znak << "(" << dijete->svojstva->redak << "," << dijete->svojstva->leks_jedinka << ")";
        else cout << dijete->svojstva->znak;
    }
    cout << endl;
    exit(0);
}

Node* provjeri_tablicu(string leks_jedinka, Tablica_Node* tablica_node){ //provjerava postoji li leks_jedinka u tablici ili njenim roditeljima
    Tablica_Node* trenutna_tablica = tablica_node;
    while(trenutna_tablica != nullptr){
        if(trenutna_tablica->zapis.find(leks_jedinka) != trenutna_tablica->zapis.end()){
            return trenutna_tablica->zapis[leks_jedinka];
        }
        trenutna_tablica = trenutna_tablica->roditelj;
    }
    return nullptr;
}

Node* provjeri_tablicu_lokalno(string leks_jedinka, Tablica_Node* tablica_node){ //provjerava postoji li leks_jedinka u trenutnoj tablici
    if(tablica_node->zapis.find(leks_jedinka) != tablica_node->zapis.end()){
        return tablica_node->zapis[leks_jedinka];
    }
    return nullptr;
}

bool provjeri_znak(const string& leks_jedinka) {
    if (leks_jedinka.length() == 3 || 
           (leks_jedinka.length() == 4 && 
            (leks_jedinka == "'\\t'" || leks_jedinka == "'\\n'" || leks_jedinka == "'\\0'" || 
             leks_jedinka == "'\\''" || leks_jedinka == "'\\\"'" || leks_jedinka == "'\\\\'"))){
                return true;
    }
    return false;
}

bool provjeri_niz_znakova(const string& leks_jedinka) { //provjerava je li niz znakova ispravno zadan
    //cout << leks_jedinka << endl;
    if (leks_jedinka.front() != '"' || leks_jedinka.back() != '"') { //mora imati " na pocetku i kraju
        return false;
    }
    bool uspio = true;
    for (size_t i = 1; i < leks_jedinka.length() - 1; ++i) {
        if (leks_jedinka[i] == '\\') { //ako imamo escape sekvencu onda gledamo sljedeci znak
            if (i + 1 < leks_jedinka.length() - 1) { // i je li jedan od znakova koji nam pase
                char sljedeci = leks_jedinka[i + 1];
                if (sljedeci != 't' && sljedeci != 'n' && sljedeci != '0' &&
                    sljedeci != '\'' && sljedeci != '\"' && sljedeci != '\\') {
                    uspio = false;
                    break;
                }
                ++i;
            } else {
                uspio = false;
                break;
            }
        }
    }
    return uspio;
}

bool moze_se_pretvoriti(string prvi, string drugi) { //provjerava moze li se tip from pretvoriti u tip to
    if (prvi == drugi) return true;
    if (prvi == "char" && (drugi == "const(char)" || drugi == "int" || drugi == "const(int)")) return true;
    if (prvi == "const(char)" && (drugi == "char" || drugi == "int" || drugi == "const(int)")) return true;
    if (prvi == "const(int)" && drugi == "int") return true;
    if (prvi == "int" && drugi == "const(int)") return true;
    if (prvi == "niz(char)" && drugi == "niz(const(char))") return true;
    if (prvi == "niz(int)" && drugi == "niz(const(int))") return true;
    return false;
}

bool jel_u_petlji(Node* node){
    while(node != nullptr){
        if(node->svojstva->znak == "<naredba_petlje>"){
            return true;
        }
        node = node->roditelj;
    }
    return false;
}

string jel_u_funkciji(Node* node){
    while(node != nullptr){
        if(node->svojstva->znak == "<definicija_funkcije>"){
            string povratni_tip = split(node->svojstva->tip, " -> ")[1];
            return povratni_tip;
        }
        node = node->roditelj;
    }
    return "";
}

void naslijedi_svojstva(Node* dijete, Node* roditelj){
    dijete->svojstva = new Node_svojstva();
    dijete->svojstva->tip = roditelj->svojstva->tip;
    dijete->svojstva->l_izraz = roditelj->svojstva->l_izraz;
    dijete->svojstva->konst = roditelj->svojstva->konst;
    dijete->svojstva->argumenti = roditelj->svojstva->argumenti;
}	

int jel_ide_u_niz_znakova(Node* node){
    int ide = 0;
    if(node->svojstva->znak == "NIZ_ZNAKOVA"){
        int broj = -1;
        for(char znak : node->svojstva->leks_jedinka){
            if(znak != '\\'){
                broj++;
            }
        }
        return broj;
    }
    if(node->djeca.empty()){
        return 0;
    }
    for(Node* dijete : node->djeca){
        ide = ide + jel_ide_u_niz_znakova(dijete);
    }
    return ide;
}

Node* jel_ide_u_identifikator(Node* node){
    if(node->svojstva->znak == "IDN"){
        //cout << node->svojstva->znak << " " << node->svojstva->leks_jedinka << " " << node->svojstva->tip << endl;
        return node;
    }
    if(node->djeca.empty()){
        
        return nullptr;
    }
    for(Node* dijete : node->djeca){
        Node* pronadeno = jel_ide_u_identifikator(dijete);
        if(pronadeno != nullptr){
            return pronadeno;
        }
    }
    return nullptr;
}

Node* provjeri_deklaracije(Tablica_Node* tablica, string ime){
    Tablica_Node* trenutna_tablica = tablica;
        for(auto it = trenutna_tablica->deklarirane_funkcije.begin(); it != trenutna_tablica->deklarirane_funkcije.end(); it++){
            if((*it)->svojstva->leks_jedinka == ime){
                return *it;
            }
        }
    return nullptr;
}

Node* provjeri_definicije(Tablica_Node* tablica, string ime){
    Tablica_Node* trenutna_tablica = tablica;
    while(trenutna_tablica != nullptr){
        for(auto it = trenutna_tablica->definirane_funkcije.begin(); it != trenutna_tablica->definirane_funkcije.end(); it++){
            if((*it)->svojstva->leks_jedinka == ime){
                return *it;
            }
        }
        trenutna_tablica = trenutna_tablica->roditelj;
    }
    return nullptr;
}


int spremi_komtekst(Node* node){
    int j = 0;
    if(registri < 5){
        for(int i = registri+1; i <= 5; i++){
            kod.push_back("\tPUSH R" + to_string(i));
            vrhStoga -= 4;
            j++;
        }
        registri = 5;
    }
    return j;
}


void obnovi_komtekst(int broj){
    for(; broj > 0; broj--){
        kod.push_back("\tPOP R" + to_string(registri-broj+1));
        registri--;
        vrhStoga += 4;
    }
}

string pretvori_u_heksadekadski(int broj) {
    stringstream ss;
    ss << "0" << hex << uppercase << broj;
    return ss.str();
}

//------------------------------------------------------------------------------------------------

void ispisi_div_funkciju() {
    string s;

    kod.push_back("\nDIV");

    // --------------
	// Spremi kontekst
    for (int i = 0; i <= 5; i++) {
        s = "\tPUSH R" + to_string(i);
        kod.push_back(s);
        vrhStoga -= 4;
    }
    // --------------
    
	kod.push_back("");
    s = "\tLOAD R1, (R7+20)";  // djeljenik
    kod.push_back(s);
    s = "\tLOAD R2, (R7+1C)";  // djelitelj
    kod.push_back(s);

    kod.push_back("\tCMP R2, 0");   // jeli djelitelj 0
    kod.push_back("\tMOVE %D 0, R6");  // UVIJEK
    kod.push_back("\tJP_Z D_KRAJ"); // Ako je djelitelj 0 preskoci petlju

    kod.push_back("D_LOOP");
    kod.push_back("\tSUB R1, R2, R1");  
    kod.push_back("\tADD R6, 1, R6");   
    kod.push_back("\tCMP R1, 0");      
    kod.push_back("\tJP_SGE D_LOOP");  

    kod.push_back("");
    kod.push_back("\tSUB R6, 1, R6");  // Umanji R6 za 1 jer smo pre�li granicu

    kod.push_back("");
    kod.push_back("D_KRAJ");
    
	// --------------
	// Obnovi kontekst
    for (int i = 5; i >= 0; i--) {
        s = "\tPOP R" + to_string(i);
        kod.push_back(s);
        vrhStoga += 4;  
    }
	// --------------
    kod.push_back("\tRET");
    
    
}


//------------------------------------------------------------------------------------------------

void primarni_izraz(Node* node, Tablica_Node* tablica_node);
void postfiks_izraz(Node* node, Tablica_Node* tablica_node);
void lista_argumenata(Node* node, Tablica_Node* tablica_node);
void unarni_izraz(Node* node, Tablica_Node* tablica_node);
void unarni_operator(Node* node, Tablica_Node* tablica_node);
void cast_izraz(Node* node, Tablica_Node* tablica_node);
void ime_tipa(Node* node, Tablica_Node* tablica_node);
void specifikator_tipa(Node* node, Tablica_Node* tablica_node);
void multiplikativni_izraz(Node* node, Tablica_Node* tablica_node);
void aditivni_izraz(Node* node, Tablica_Node* tablica_node);
void odnosni_izraz(Node* node, Tablica_Node* tablica_node);
void jednakosni_izraz(Node* node, Tablica_Node* tablica_node);
void bin_i_izraz(Node* node, Tablica_Node* tablica_node);
void bin_xili_izraz(Node* node, Tablica_Node* tablica_node);
void bin_ili_izraz(Node* node, Tablica_Node* tablica_node);
void log_i_izraz(Node* node, Tablica_Node* tablica_node);
void log_ili_izraz(Node* node, Tablica_Node* tablica_node);
void izraz_pridruzivanja(Node* node, Tablica_Node* tablica_node);
void izraz(Node* node, Tablica_Node* tablica_node);
void slozena_naredba(Node* node, Tablica_Node* tablica_node);
void lista_naredbi(Node* node, Tablica_Node* tablica_node);
void naredba(Node* node, Tablica_Node* tablica_node);
void izraz_naredba(Node* node, Tablica_Node* tablica_node);
void naredba_grananja(Node* node, Tablica_Node* tablica_node);
void naredba_petlje(Node* node, Tablica_Node* tablica_node);
void naredba_skoka(Node* node, Tablica_Node* tablica_node);
void prijevodna_jedinica(Node* node, Tablica_Node* tablica_node);
void vanjska_deklaracija(Node* node, Tablica_Node* tablica_node);
void definicija_funkcije(Node* node, Tablica_Node* tablica_node);
void lista_parametara(Node* node, Tablica_Node* tablica_node);
void deklaracija_parametra(Node* node, Tablica_Node* tablica_node);
void lista_deklaracija(Node* node, Tablica_Node* tablica_node);
void deklaracija(Node* node, Tablica_Node* tablica_node);
void lista_init_deklaratora(Node* node, Tablica_Node* tablica_node);
void init_deklarator(Node* node, Tablica_Node* tablica_node);
void izravni_deklarator(Node* node, Tablica_Node* tablica_node);
void inicijalizator(Node* node, Tablica_Node* tablica_node);
void lista_izraza_pridruzivanja(Node* node, Tablica_Node* tablica_node);



void primarni_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);
    if(node->djeca.size() == 1){
        if(node->djeca[0]->svojstva->znak == "IDN"){ //ako je dijete IDN
        Node* pronadeno = provjeri_tablicu(node->djeca[0]->svojstva->leks_jedinka, tablica_node);
            if(pronadeno == nullptr){ //ako ne postoji u tablici
                greska(node);
            }
            else{ //ako postoji u tablici, postavi svojstva
                node->svojstva->tip = pronadeno->svojstva->tip;
                node->svojstva->l_izraz = pronadeno->svojstva->l_izraz;
                node->svojstva->konst = pronadeno->svojstva->konst;
                node->svojstva->argumenti = pronadeno->svojstva->argumenti;

                node->djeca[0]->svojstva->tip = pronadeno->svojstva->tip;
                node->djeca[0]->svojstva->l_izraz = pronadeno->svojstva->l_izraz;
                node->djeca[0]->svojstva->konst = pronadeno->svojstva->konst;
                node->djeca[0]->svojstva->argumenti = pronadeno->svojstva->argumenti;
            }
            
            //----------------------------------------------------------------------
            /*if (aktivnaNaredbaSkoka) { // Ako smo dosli do izraza u aktivnoj naredbi skoka mora biti da je return;
            	string s = "\tLOAD R6, (" + adresa[node->djeca[0]->svojstva->leks_jedinka] + ")"; //#ret
            	kod.push_back(s);            	
			}*/
            //----------------------------------------------------------------------

            // idemo spremiti varijablu u R6 i neki registar

            /*if (aktivnaDeklaracija && isGlobal 
            &&  trenutniIzravniDeklarator->djeca[0]->svojstva->leks_jedinka != node->djeca[0]->svojstva->leks_jedinka) { // Kao do�li smo do broja tokom deklaracije
                string identifikator = trenutniIzravniDeklarator->djeca[0]->svojstva->leks_jedinka;
            	adresa[node->djeca[0]->svojstva->leks_jedinka] = adresa[identifikator]; //#ret
                return;
			}

            if (aktivnaDeklaracija && isGlobal 
            &&  trenutniIzravniDeklarator->djeca[0]->svojstva->leks_jedinka == node->djeca[0]->svojstva->leks_jedinka) { // Kao do�li smo do broja tokom deklaracije
                string identifikator = trenutniIzravniDeklarator->djeca[0]->svojstva->leks_jedinka;
                string s = "G_" + identifikator + "\tDW %D " + "1234555"; //garbage value (nasumicni)
            	kod.push_back(s);
            		
            	adresa[identifikator] = "G_" + identifikator;
			}*/

            if(node->djeca[0]->svojstva->tip.substr(0, 8) != "funkcija" && !neUzimajAdresu){
                Tablica_Node* trenutna_tablica = tablica_node;
                int i = 0;
                string znakic = node->djeca[0]->svojstva->leks_jedinka;
                if(node->roditelj->roditelj->djeca.size() > 1 && node->roditelj->roditelj->djeca[1]->svojstva->leks_jedinka == "["){
                    while(trenutna_tablica != nullptr){
                        i += trenutna_tablica->adresa_na_stogu.size();
                        if(trenutna_tablica->roditelj == nullptr){
                            string s;
                            s = "\tSHL R6, 2, R6";
                            kod.push_back(s);
                            s = "\tMOVE " + adresa[node->djeca[0]->svojstva->leks_jedinka+"z0z"] + ", R" + to_string(registri);
                            kod.push_back(s);
                            s = "\tADD R" + to_string(registri) + ", R6, R6";
                            kod.push_back(s);
                            s = "\tLOAD R6, (R6)";
                            kod.push_back(s);
                            break;
                        }
                        if(trenutna_tablica->adresa_na_stogu.find(node->djeca[0]->svojstva->leks_jedinka) != trenutna_tablica->adresa_na_stogu.end()){
                            string s;
                            s = "\tSHL R6, 2, R6";
                            kod.push_back(s);
                            int pozicija = (i)*4 - trenutna_tablica->adresa_na_stogu[node->djeca[0]->svojstva->leks_jedinka] + brojPusheva*4;
                            s = "\tADD R6, " + pretvori_u_heksadekadski(pozicija) + ", R6";
                            kod.push_back(s);
                            s = "\tLOAD R6, (R7+R6)";
                            kod.push_back(s);
                            break;
                        }
                        trenutna_tablica = trenutna_tablica->roditelj;
                    }
                    return;
                }
                while(trenutna_tablica != nullptr){
                    i += trenutna_tablica->adresa_na_stogu.size();
                    if(trenutna_tablica->roditelj == nullptr){
                        string s = "\tLOAD R6, (" + adresa[node->djeca[0]->svojstva->leks_jedinka] + ")";
                        kod.push_back(s);
                        break;
                    }
                    if(trenutna_tablica->adresa_na_stogu.find(node->djeca[0]->svojstva->leks_jedinka) != trenutna_tablica->adresa_na_stogu.end()){
                        string s;
                        int pozicija = (i)*4 - trenutna_tablica->adresa_na_stogu[node->djeca[0]->svojstva->leks_jedinka] + brojPusheva*4;
                        if(pozicija < 0){
                            s = "\tLOAD R6, (R7" + pretvori_u_heksadekadski(pozicija) + ")";
                        }
                        else s = "\tLOAD R6, (R7+" + pretvori_u_heksadekadski(pozicija) + ")";
                        kod.push_back(s);
                        break;
                    }
                    trenutna_tablica = trenutna_tablica->roditelj;
                }
            }
        }
        else if(node->djeca[0]->svojstva->znak == "BROJ"){ //ako je dijete BROJ
            string broj_str = node->djeca[0]->svojstva->leks_jedinka;
            long long broj;
            if(broj_str.size() > 12){
                greska(node);
            }
            if(broj_str.size() > 2 && broj_str[0] == '0' && (broj_str[1] == 'x' || broj_str[1] == 'X')){ // heksadekadski broj
                broj = stoll(broj_str, nullptr, 16);
            } else { // decimalni broj
                broj = stoll(broj_str);
            }
            if(node->roditelj->roditelj->roditelj->roditelj->svojstva->znak == "<unarni_izraz>"
            && node->roditelj->roditelj->roditelj->roditelj->djeca[0]->djeca[0]->svojstva->znak == "MINUS"){
                broj_str  = "-" + broj_str;
                if(broj-1 > INT_MAX){
                    greska(node);
                }
            }
            else if(!(broj <= INT_MAX && broj >= INT_MIN)){
                greska(node);
            }
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;

            node->djeca[0]->svojstva->tip = "int";
            node->djeca[0]->svojstva->l_izraz = false;

            // -----------------------------------------------------------------------
			
			/*if (aktivnaDeklaracija && isGlobal) { // Kao do�li smo do broja tokom deklaracije
                                                // Definicija globalne varijable, trazi #gl
                    if(trenutniArray != ""){
                        string identifikator = trenutniArray + "z" + to_string(brojPushevaArray) + "z";
                        string s = "G_" + identifikator + "\tDW %D " + broj_str;
                        kod.push_back(s);
                        
                        adresa[identifikator] = "G_" + identifikator; //#ret
                        brojPushevaArray += 4;
                        return;
                    }
            	    string identifikator = trenutniIzravniDeklarator->djeca[0]->svojstva->leks_jedinka;
            		string s = "G_" + identifikator + "\tDW %D " + broj_str;
            		kod.push_back(s);
            		
            		adresa[identifikator] = "G_" + identifikator; //#ret
			} else */{
            // -----------------------------------------------------------------------
        		if ( stoi(broj_str) > 79999 ) {
        			brojacVelikihBrojeva ++;
        			string s = "G_" + to_string(brojacVelikihBrojeva) + "\tDW %D " + broj_str;
				    
					adresa[to_string(brojacVelikihBrojeva)] = "G_" + to_string(brojacVelikihBrojeva); //#vb
				    
					kod.push_back(s);
					
				//	s = "\tLOAD R" + to_string(registri) + ", (G_" + to_string(brojacVelikihBrojeva) +")";
		        //  kod.push_back(s);
		            
					s = "\tLOAD R6, (G_" + to_string(brojacVelikihBrojeva) +")";
		            kod.push_back(s);
		            
		            registri--;
		            
				} else {
					string s = "\tMOVE %D " + broj_str + ", R6";
		            kod.push_back(s);
		            //s = "\tMOVE %D " + broj_str + ", R" + to_string(registri);
		            //registri--;
		            //kod.push_back(s);
            	}
			}
        }
        else if(node->djeca[0]->svojstva->znak == "ZNAK"){ //ako je dijete ZNAK
            if(provjeri_znak(node->djeca[0]->svojstva->leks_jedinka)){ //ako je ispravan znak, postavi svojstva
                node->svojstva->tip = "char";
                node->svojstva->l_izraz = false;
            } else {
                greska(node);
            }
            string broj_str = to_string((int)node->djeca[0]->svojstva->leks_jedinka[1]);
            /*if (aktivnaDeklaracija && isGlobal) { // Kao do�li smo do broja tokom deklaracije
                                                // Definicija globalne varijable, trazi #gl
                    if(trenutniArray != ""){
                        string identifikator = trenutniArray + "z" + to_string(brojPushevaArray) + "z";
                        string s = "G_" + identifikator + "\tDW %D " + broj_str;
                        kod.push_back(s);
                        
                        adresa[identifikator] = "G_" + identifikator; //#ret
                        brojPushevaArray += 4;
                        return;
                    }
            	    string identifikator = trenutniIzravniDeklarator->djeca[0]->svojstva->leks_jedinka;
            	    
            		string s = "G_" + identifikator + "\tDW %D " + broj_str;
            		kod.push_back(s);
            		
            		adresa[identifikator] = "G_" + identifikator; //#ret
			} else */{
            // -----------------------------------------------------------------------
        		if ( stoi(broj_str) > 79999 ) {
        			brojacVelikihBrojeva ++;
        			string s = "G_" + to_string(brojacVelikihBrojeva) + "\tDW %D " + broj_str;
				    
					adresa[to_string(brojacVelikihBrojeva)] = "G_" + to_string(brojacVelikihBrojeva); //#vb
				    
					kod.push_back(s);
					s = "\tLOAD R6, (G_" + to_string(brojacVelikihBrojeva) +")";
		            kod.push_back(s);
		            
		            registri--;
		            
				} else {
					string s = "\tMOVE %D " + broj_str + ", R6";
		            kod.push_back(s);
            	}
			}
        }
        else if(node->djeca[0]->svojstva->znak == "NIZ_ZNAKOVA"){ //ako je dijete NIZ_ZNAKOVA
            if(provjeri_niz_znakova(node->djeca[0]->svojstva->leks_jedinka)){ //ako je ispravan niz znakova, postavi svojstva
                node->svojstva->tip = "niz(const(char))";
                node->svojstva->l_izraz = false;
            } else {
                greska(node);
            }
        }
        else{
            greska(node);
        }
    }

    //ako su djeca L_ZAGRADA <izraz> D_ZAGRADA
    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "L_ZAGRADA" 
    && node->djeca[2]->svojstva->znak == "D_ZAGRADA" && node->djeca[1]->svojstva->znak == "<izraz>"){
        izraz(node->djeca[1], tablica_node); //provjeri izraz
        node->svojstva->tip = node->djeca[1]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[1]->svojstva->l_izraz;
    }
    else{
        greska(node);
        
    }
}

void postfiks_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    //ako je dijete <primarni_izraz>
    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<primarni_izraz>"){
        primarni_izraz(node->djeca[0], tablica_node); //provjeri primarni izraz
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
        node->svojstva->argumenti = node->djeca[0]->svojstva->argumenti;
    }

    else if(node->djeca.size() == 4 && node->djeca[0]->svojstva->znak == "<postfiks_izraz>" 
    && node->djeca[1]->svojstva->znak == "L_UGL_ZAGRADA" && node->djeca[2]->svojstva->znak == "<izraz>"
    && node->djeca[3]->svojstva->znak == "D_UGL_ZAGRADA"){
        izraz(node->djeca[2], tablica_node);
        postfiks_izraz(node->djeca[0], tablica_node);
        if(node->djeca[0]->svojstva->tip.substr(0, 3) != "niz"){
            greska(node);
        }
        else{ //ako se radi o nizu
            string podtip = node->djeca[0]->svojstva->tip.substr(4, node->djeca[0]->svojstva->tip.size()-5);
            if(podtip.substr(0, 5) == "const"){
                node->svojstva->tip = podtip;
                node->svojstva->l_izraz = false;
            }
            else{
                node->svojstva->tip = podtip;
                node->svojstva->l_izraz = true;
            }
        }

        if (!moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")) {
            greska(node);
        }
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<postfiks_izraz>" 
    && node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "D_ZAGRADA"){
        postfiks_izraz(node->djeca[0], tablica_node);
        int i = spremi_komtekst(node);
        kod.push_back("\tCALL F_" + node->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka);
        obnovi_komtekst(i);
        if(node->djeca[0]->svojstva->tip.substr(0, 8) != "funkcija"){
            greska(node);
        }
        else{
            if(!node->djeca[0]->svojstva->argumenti.empty()){
                greska(node);
            }
            else{
                string povratni_tip = split(node->djeca[0]->svojstva->tip, " -> ")[1];
                node->svojstva->tip = povratni_tip;
                node->svojstva->l_izraz = false;
            }
        }
    }

    else if(node->djeca.size() == 4 && node->djeca[0]->svojstva->znak == "<postfiks_izraz>" && 
    node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "<lista_argumenata>"
    && node->djeca[3]->svojstva->znak == "D_ZAGRADA"){
        postfiks_izraz(node->djeca[0], tablica_node);
        int i = spremi_komtekst(node);
        lista_argumenata(node->djeca[2], tablica_node);
        string funkcija_ime = node->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka;
        kod.push_back("\tCALL F_" + funkcija_ime);
        int j = node->djeca[0]->svojstva->argumenti.size();
        kod.push_back("\tADD R7, " + pretvori_u_heksadekadski(j*4) + ", R7"); //micemo argumente sa stoga
        vrhStoga += j*4;
        obnovi_komtekst(i);
        if(node->djeca[0]->svojstva->tip.substr(0, 8) != "funkcija"){
            greska(node);
        }
        else{
            if(node->djeca[0]->svojstva->argumenti.empty()){
                greska(node);
            }
            else{
                if(node->djeca[0]->svojstva->argumenti.size() != node->djeca[2]->svojstva->argumenti.size()){
                    greska(node);
                }
                for(int i = 0; i < node->djeca[0]->svojstva->argumenti.size(); i++){
                    if(!moze_se_pretvoriti(node->djeca[2]->svojstva->argumenti[i], node->djeca[0]->svojstva->argumenti[i])){
                        greska(node);
                    }
                }
                string povratni_tip = split(node->djeca[0]->svojstva->tip, " -> ")[1];
                node->svojstva->tip = povratni_tip;
                node->svojstva->l_izraz = false;
            }
        }
    }

    else if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "<postfiks_izraz>"
    && (node->djeca[1]->svojstva->znak == "OP_INC" || node->djeca[1]->svojstva->znak == "OP_DEC")){
        postfiks_izraz(node->djeca[0], tablica_node);

        string s;
        if(node->djeca[1]->svojstva->znak == "OP_INC"){
            s  = "\tMOVE R6, R" + to_string(registri);
            kod.push_back(s);
            s = "\tADD R" + to_string(registri) + ", %D 1, R" + to_string(registri);
            kod.push_back(s);
        }
        else{
            s  = "\tMOVE R6, R" + to_string(registri);
            kod.push_back(s);
            s = "\tSUB R" + to_string(registri) + ", %D 1, R" + to_string(registri);
            kod.push_back(s);
        }

        if(node->djeca[0]->djeca[0]->djeca.size() == 1 && node->djeca[0]->djeca[0]->djeca[0]->svojstva->znak == "IDN"){
            Tablica_Node* trenutna_tablica = tablica_node;
            
                int i = 0;
                while(trenutna_tablica != nullptr){
                    i += trenutna_tablica->adresa_na_stogu.size();
                    if(trenutna_tablica->roditelj == nullptr){
                        string s = "\tSTORE R" + to_string(registri) + ", (" + adresa[node->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka] + ")";
                        kod.push_back(s);
                        //string z = "\tLOAD R" + to_string(registri) + ", (" + adresa[node->djeca[0]->svojstva->leks_jedinka] + ")";
                        //registri--;
                        //kod.push_back(z);
                        break;
                    }
                    if(trenutna_tablica->adresa_na_stogu.find(node->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka) != trenutna_tablica->adresa_na_stogu.end()){
                        string s;
                        int pozicija = (i)*4 - trenutna_tablica->adresa_na_stogu[node->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka] + brojPusheva*4;
                        //cout << trenutna_tablica->adresa_na_stogu[node->djeca[0]->svojstva->leks_jedinka] << endl;
                        if(pozicija < 0){
                            s = "\tSTORE R" + to_string(registri) + ", (R7" + pretvori_u_heksadekadski(pozicija) + ")";
                        }
                        else s = "\tSTORE R" + to_string(registri) + ", (R7+" + pretvori_u_heksadekadski(pozicija) + ")";
                        kod.push_back(s);
                        //string z = "\tLOAD R" + to_string(registri) + ", (R7+" + to_string(i*4 - trenutna_tablica->adresa_na_stogu[node->djeca[0]->svojstva->leks_jedinka]) + ")";
                        //registri--;
                        //kod.push_back(z);
                        break;
                    }
                    trenutna_tablica = trenutna_tablica->roditelj;
                }
        }

        if(node->djeca[0]->svojstva->l_izraz == false || !moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int")){
            greska(node);
        }
        node->svojstva->tip = "int";
        node->svojstva->l_izraz = false;
    }

    else{
        greska(node);
    }
}

void lista_argumenata(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);
    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<izraz_pridruzivanja>"){
        izraz_pridruzivanja(node->djeca[0], tablica_node);
        node->svojstva->argumenti.push_back(node->djeca[0]->svojstva->tip);
        kod.push_back("\tPUSH R6");
        vrhStoga -= 4;
        registri = 5;
    }
    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<lista_argumenata>"
    && node->djeca[1]->svojstva->znak == "ZAREZ" && node->djeca[2]->svojstva->znak == "<izraz_pridruzivanja>"){
        lista_argumenata(node->djeca[0], tablica_node);
        izraz_pridruzivanja(node->djeca[2], tablica_node);
        node->svojstva->argumenti = node->djeca[0]->svojstva->argumenti;
        node->svojstva->argumenti.push_back(node->djeca[2]->svojstva->tip);
        kod.push_back("\tPUSH R6");
        vrhStoga -= 4;
        registri = 5;
    }
    else{
        greska(node);
    }
}

void unarni_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<postfiks_izraz>"){
        postfiks_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 2 && (node->djeca[0]->svojstva->znak == "OP_INC" ||node->djeca[0]->svojstva->znak == "OP_DEC") 
    && node->djeca[1]->svojstva->znak == "<unarni_izraz>"){
        unarni_izraz(node->djeca[1], tablica_node);

        string s;
        if(node->djeca[0]->svojstva->znak == "OP_INC"){
            s  = "\tADD R6, 1, R6";
            kod.push_back(s);
        }
        else{
            s  = "\tSUB R6, 1, R6";
            kod.push_back(s);
        }


        if(node->djeca[1]->djeca[0]->djeca[0]->djeca.size() == 1 && node->djeca[1]->djeca[0]->djeca[0]->djeca[0]->svojstva->znak == "IDN"){
            Tablica_Node* trenutna_tablica = tablica_node;
            
                int i = 0;
                while(trenutna_tablica != nullptr){
                    i += trenutna_tablica->adresa_na_stogu.size();
                    if(trenutna_tablica->roditelj == nullptr){
                        string s = "\tSTORE R6, (" + adresa[node->djeca[1]->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka] + ")";
                        kod.push_back(s);
                        //string z = "\tLOAD R" + to_string(registri) + ", (" + adresa[node->djeca[0]->svojstva->leks_jedinka] + ")";
                        //registri--;
                        //kod.push_back(z);
                        break;
                    }
                    if(trenutna_tablica->adresa_na_stogu.find(node->djeca[1]->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka) != trenutna_tablica->adresa_na_stogu.end()){
                        string s;
                        int pozicija = (i)*4 - trenutna_tablica->adresa_na_stogu[node->djeca[1]->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka] + brojPusheva*4;
                        //cout << trenutna_tablica->adresa_na_stogu[node->djeca[0]->svojstva->leks_jedinka] << endl;
                        if(pozicija < 0){
                            s = "\tSTORE R6, (R7" + pretvori_u_heksadekadski(pozicija) + ")";
                        }
                        else s = "\tSTORE R6, (R7+" + pretvori_u_heksadekadski(pozicija) + ")";
                        kod.push_back(s);
                        //string z = "\tLOAD R" + to_string(registri) + ", (R7+" + to_string(i*4 - trenutna_tablica->adresa_na_stogu[node->djeca[0]->svojstva->leks_jedinka]) + ")";
                        //registri--;
                        //kod.push_back(z);
                        break;
                    }
                    trenutna_tablica = trenutna_tablica->roditelj;
                }
        }

        if(node->djeca[1]->svojstva->l_izraz == false || !moze_se_pretvoriti(node->djeca[1]->svojstva->tip, "int")){
            greska(node);
        }
        node->svojstva->tip = "int";
        node->svojstva->l_izraz = false;
    }

    else if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "<unarni_operator>"
    && node->djeca[1]->svojstva->znak == "<cast_izraz>"){
        cast_izraz(node->djeca[1], tablica_node);
        unarni_operator(node->djeca[0], tablica_node);
        if(moze_se_pretvoriti(node->djeca[1]->svojstva->tip, "int")){
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
        else{
            greska(node);
        }
    }
}

void unarni_operator(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && (node->djeca[0]->svojstva->znak == "PLUS" || node->djeca[0]->svojstva->znak == "MINUS"
    || node->djeca[0]->svojstva->znak == "OP_TILDA" || node->djeca[0]->svojstva->znak == "OP_NEG")){
        node->svojstva->tip = "int"; //mozda nije potrebno
        node->svojstva->l_izraz = false;
    }

    else{
        greska(node);
    }
}

void cast_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<unarni_izraz>"){
        unarni_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 4 && node->djeca[0]->svojstva->znak == "L_ZAGRADA" 
    && node->djeca[1]->svojstva->znak == "<ime_tipa>" && node->djeca[2]->svojstva->znak == "D_ZAGRADA"
    && node->djeca[3]->svojstva->znak == "<cast_izraz>"){
        ime_tipa(node->djeca[1], tablica_node);
        cast_izraz(node->djeca[3], tablica_node);
        
        if(moze_se_pretvoriti(node->djeca[3]->svojstva->tip, node->djeca[1]->svojstva->tip)
        || (node->djeca[3]->svojstva->tip == "int" && node->djeca[1]->svojstva->tip == "char")
        || (node->djeca[3]->svojstva->tip == "const(int)" && node->djeca[1]->svojstva->tip == "const(char)")
        || (node->djeca[3]->svojstva->tip == "int" && node->djeca[1]->svojstva->tip == "const(char)")
        || (node->djeca[3]->svojstva->tip == "const(int)" && node->djeca[1]->svojstva->tip == "char")){
            node->svojstva->tip = node->djeca[1]->svojstva->tip;
            node->svojstva->l_izraz = false;
        }
        else{
            //cout << node->djeca[1]->svojstva->tip << " " << node->djeca[3]->svojstva->tip << endl;
            greska(node);
        }
    }

    else{
        greska(node);
    }
}

void ime_tipa(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<specifikator_tipa>"){
        specifikator_tipa(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
    }

    else if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "KR_CONST" 
    && node->djeca[1]->svojstva->znak == "<specifikator_tipa>"){
        specifikator_tipa(node->djeca[1], tablica_node);
        if(node->djeca[1]->svojstva->tip == "void"){
            greska(node);
        }
        node->svojstva->tip = "const(" + node->djeca[1]->svojstva->tip + ")";
        node->svojstva->konst = true;
    }

    else{
        greska(node);
    }
}

void specifikator_tipa(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "KR_VOID"){
        node->svojstva->tip = "void";
    }

    else if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "KR_INT"){
        node->svojstva->tip = "int";
    }

    else if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "KR_CHAR"){
        node->svojstva->tip = "char";
    }

    else{
        greska(node);
    }
}

void multiplikativni_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<cast_izraz>"){
        cast_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<multiplikativni_izraz>" 
    && (node->djeca[1]->svojstva->znak == "OP_PUTA" || node->djeca[1]->svojstva->znak == "OP_DIJELI" 
    || node->djeca[1]->svojstva->znak == "OP_MOD") && node->djeca[2]->svojstva->znak == "<cast_izraz>"){
        multiplikativni_izraz(node->djeca[0], tablica_node);
        cast_izraz(node->djeca[2], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska(node);
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska(node);
    }
}

void aditivni_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if (node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<multiplikativni_izraz>") {
    
	    multiplikativni_izraz(node->djeca[0], tablica_node);
	
	    // Postavi svojstva trenutnog �vora
	    node->svojstva->tip = node->djeca[0]->svojstva->tip;
	    node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
	
		//---------------------------------------
	    // Generiranje koda samo za prvi operand

		//---------------------------------------
	}


    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<aditivni_izraz>" 
    && (node->djeca[1]->svojstva->znak == "PLUS" || node->djeca[1]->svojstva->znak == "MINUS")
    && node->djeca[2]->svojstva->znak == "<multiplikativni_izraz>"){

        string s;
		aditivni_izraz(node->djeca[0], tablica_node);
        
        s = "\tMOVE R6, R" + to_string(registri);
	    kod.push_back(s);   
        s = "\tPUSH R" + to_string(registri);
        kod.push_back(s);
        brojPusheva++;

        multiplikativni_izraz(node->djeca[2], tablica_node); // recentMultiplikativniIzraz trebao bi biti u R6 jer je sve u R6
        
        s = "\tPOP R" + to_string(registri);
        brojPusheva--;
        registri--;
	    kod.push_back(s);
        s = "\tMOVE R6, R" + to_string(registri);
        kod.push_back(s);
        registri++;

        
        //------------------------------------------------------------------
        // basically nadodaj Multiplikativni u R5 i spremi u R6
        // rezultat funkcije je u R6
        
        
        
		if (node->djeca[1]->svojstva->znak == "PLUS") { // dodaj multiplikativni izraz koji je izracunat u registru R6
			s = "\tADD R" + to_string(registri) + ", R" + to_string(registri-1) + ", R6"; // sta god da se izracunalo stavi u R5 da R6 bude slobodan ako dode funkcija
	        kod.push_back(s);  
    	}
    	else { // mora da je MINUS
    		s = "\tSUB R" + to_string(registri) + ", R" + to_string(registri-1) + ", R6" ; // sta god da se izracunalo stavi u R5 da R6 bude slobodan ako dode funkcija
	        kod.push_back(s);
		}
		
        //------------------------------------------------------------------        
        
		if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            //cout << node->djeca[0]->svojstva->tip << " " << node->djeca[2]->svojstva->tip << endl;
            greska(node);
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska(node);
    }
}

void odnosni_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<aditivni_izraz>"){
        aditivni_izraz(node->djeca[0], tablica_node);
        //cout << node->djeca[0]->svojstva->tip << endl;
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<odnosni_izraz>" 
    && (node->djeca[1]->svojstva->znak == "OP_LT" || node->djeca[1]->svojstva->znak == "OP_GT" 
    || node->djeca[1]->svojstva->znak == "OP_LTE" || node->djeca[1]->svojstva->znak == "OP_GTE")
    && node->djeca[2]->svojstva->znak == "<aditivni_izraz>"){
        odnosni_izraz(node->djeca[0], tablica_node);

        string s;
        s = "\tMOVE R6, R" + to_string(registri);
	    kod.push_back(s);   
        s = "\tPUSH R" + to_string(registri);
        kod.push_back(s);
        brojPusheva++;


        aditivni_izraz(node->djeca[2], tablica_node);

        s = "\tPOP R" + to_string(registri);
        brojPusheva--;
        registri--;
	    kod.push_back(s);
        s = "\tMOVE R6, R" + to_string(registri);
        kod.push_back(s);
        registri++;

        int i = elseLabel;
        elseLabel++;

        if (node->djeca[1]->svojstva->znak == "OP_LT") {
			s = "\tCMP R" + to_string(registri) + ", R" + to_string(registri-1);
	        kod.push_back(s);  
            s = "\tMOVE 1, R6";
            kod.push_back(s);
            s = "\tJR_SLT ODN_" + to_string(i);
            kod.push_back(s);
    	}
    	else if (node->djeca[1]->svojstva->znak == "OP_GT") {
			s = "\tCMP R" + to_string(registri) + ", R" + to_string(registri-1); 
	        kod.push_back(s);  
            s = "\tMOVE 1, R6";
            kod.push_back(s);
            s = "\tJR_SGT ODN_" + to_string(i);
            kod.push_back(s);
    	}
        else if (node->djeca[1]->svojstva->znak == "OP_GTE") {
			s = "\tCMP R" + to_string(registri) + ", R" + to_string(registri-1); 
	        kod.push_back(s);  
            s = "\tMOVE 1, R6";
            kod.push_back(s);
            s = "\tJR_SGE ODN_" + to_string(i);
            kod.push_back(s);
    	}
        else if (node->djeca[1]->svojstva->znak == "OP_LTE") {
			s = "\tCMP R" + to_string(registri) + ", R" + to_string(registri-1); 
	        kod.push_back(s);  
            s = "\tMOVE 1, R6";
            kod.push_back(s);
            s = "\tJR_SLE ODN_" + to_string(i);
            kod.push_back(s);
    	}

        s = "\tMOVE 0, R6";
        kod.push_back(s);
        s = "\tJR ODN_" + to_string(i);
        kod.push_back(s);
        s = "ODN_" + to_string(i);
        kod.push_back(s);
        


        if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska(node);
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska(node);
    }
}

void jednakosni_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<odnosni_izraz>"){
        odnosni_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<jednakosni_izraz>" 
    && (node->djeca[1]->svojstva->znak == "OP_EQ" || node->djeca[1]->svojstva->znak == "OP_NEQ")
    && node->djeca[2]->svojstva->znak == "<odnosni_izraz>"){
        jednakosni_izraz(node->djeca[0], tablica_node);

        string s;
        s = "\tMOVE R6, R" + to_string(registri);
	    kod.push_back(s);   
        s = "\tPUSH R" + to_string(registri);
        kod.push_back(s);
        brojPusheva++;

        odnosni_izraz(node->djeca[2], tablica_node);

        s = "\tPOP R" + to_string(registri);
        brojPusheva--;
        registri--;
	    kod.push_back(s);
        s = "\tMOVE R6, R" + to_string(registri);
        kod.push_back(s);
        registri++;

        int i = elseLabel;
        elseLabel++;

        if (node->djeca[1]->svojstva->znak == "OP_EQ") {
			s = "\tCMP R" + to_string(registri) + ", R" + to_string(registri-1);
	        kod.push_back(s);  
            s = "\tMOVE 1, R6";
            kod.push_back(s);
            s = "\tJR_EQ ODN_" + to_string(i);
            kod.push_back(s);
    	}
        else if (node->djeca[1]->svojstva->znak == "OP_NEQ") {
			s = "\tCMP R" + to_string(registri) + ", R" + to_string(registri-1);
	        kod.push_back(s);  
            s = "\tMOVE 1, R6";
            kod.push_back(s);
            s = "\tJR_NE ODN_" + to_string(i);
            kod.push_back(s);
    	}

        s = "\tMOVE 0, R6";
        kod.push_back(s);
        s = "\tJR ODN_" + to_string(i);
        kod.push_back(s);
        s = "ODN_" + to_string(i);
        kod.push_back(s);


        if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska(node);
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska(node);
    }
}

void bin_i_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<jednakosni_izraz>"){
        jednakosni_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
        
    /*
    	//----------------------------------
    	// Generiranje koda za prvi operand
        string s = "\tMOVE R6, R" + to_string(registri);
        kod.push_back(s);
    	//----------------------------------
    */
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<bin_i_izraz>" 
    && node->djeca[1]->svojstva->znak == "OP_BIN_I" && node->djeca[2]->svojstva->znak == "<jednakosni_izraz>"){
        bin_i_izraz(node->djeca[0], tablica_node);

        string s;
        s = "\tMOVE R6, R" + to_string(registri);
	    kod.push_back(s);   
        s = "\tPUSH R" + to_string(registri);
        kod.push_back(s);
        brojPusheva++;
        
        jednakosni_izraz(node->djeca[2], tablica_node);

        s = "\tPOP R" + to_string(registri);
        brojPusheva--;
        registri--;
	    kod.push_back(s);
        s = "\tMOVE R6, R" + to_string(registri);
        kod.push_back(s);
        registri++;


        s = "\tAND R" + to_string(registri) + ", R" + to_string(registri-1) + ", R6"; // sta god da se izracunalo stavi u R5 da R6 bude slobodan ako dode funkcija
	    kod.push_back(s); 
        
		if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska(node);
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
        
        /*
        //------------------------------------------------------------------
        
        
		s = "\tAND R" + to_string(registri) + ", R6" + ", R" +to_string(registri); 
        kod.push_back(s);
        s = "\tMOVE R" + to_string(registri) + ", R6";
        kod.push_back(s);   		
        //------------------------------------------------------------------
        */
    }

    else{
        greska(node);
    }
}

void bin_xili_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<bin_i_izraz>"){
        bin_i_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;

        /*  
    	//----------------------------------
    	// Generiranje koda za prvi operand
        string s = "\tMOVE R6, R" + to_string(registri);
        kod.push_back(s);
    	//----------------------------------
    	*/
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<bin_xili_izraz>" 
    && node->djeca[1]->svojstva->znak == "OP_BIN_XILI" && node->djeca[2]->svojstva->znak == "<bin_i_izraz>"){
        bin_xili_izraz(node->djeca[0], tablica_node);

        string s;
        s = "\tMOVE R6, R" + to_string(registri);
	    kod.push_back(s);   
        s = "\tPUSH R" + to_string(registri);
        kod.push_back(s);
        brojPusheva++;

        bin_i_izraz(node->djeca[2], tablica_node);

        s = "\tPOP R" + to_string(registri);
        brojPusheva--;
        registri--;
	    kod.push_back(s);
        s = "\tMOVE R6, R" + to_string(registri);
        kod.push_back(s);
        registri++;


        if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska(node);
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
        
        //------------------------------------------------------------------
		s = "\tXOR R" + to_string(registri) + ", R" + to_string(registri-1) + ", R6"; // sta god da se izracunalo stavi u R5 da R6 bude slobodan ako dode funkcija
	    kod.push_back(s);   		
        //------------------------------------------------------------------
    }

    else{
        greska(node);
    }
}

void bin_ili_izraz(Node* node, Tablica_Node* tablica_node){
    
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<bin_xili_izraz>"){
        bin_xili_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    
    /*
    	//----------------------------------
    	// Generiranje koda za prvi operand
        string s = "\tMOVE R6, R" + to_string(registri);
        kod.push_back(s);
    	//----------------------------------
    */
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<bin_ili_izraz>" 
    && node->djeca[1]->svojstva->znak == "OP_BIN_ILI" && node->djeca[2]->svojstva->znak == "<bin_xili_izraz>"){
        bin_ili_izraz(node->djeca[0], tablica_node);

        string s;
        s = "\tMOVE R6, R" + to_string(registri);
	    kod.push_back(s);   
        s = "\tPUSH R" + to_string(registri);
        kod.push_back(s);
        brojPusheva++;

        bin_xili_izraz(node->djeca[2], tablica_node);

        s = "\tPOP R" + to_string(registri);
        brojPusheva--;
        registri--;
	    kod.push_back(s);
        s = "\tMOVE R6, R" + to_string(registri);
        kod.push_back(s);
        registri++;

        if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska(node);
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
        
    	//------------------------------------------------------------------
        s = "\tOR R" + to_string(registri) + ", R" + to_string(registri-1) + ", R6"; // sta god da se izracunalo stavi u R5 da R6 bude slobodan ako dode funkcija
	    kod.push_back(s); 		
        //------------------------------------------------------------------
    }

    else{
        greska(node);
    }
}

void log_i_izraz(Node* node, Tablica_Node* tablica_node){
    
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<bin_ili_izraz>"){
        bin_ili_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<log_i_izraz>" 
    && node->djeca[1]->svojstva->znak == "OP_I" && node->djeca[2]->svojstva->znak == "<bin_ili_izraz>"){
        log_i_izraz(node->djeca[0], tablica_node);
        bin_ili_izraz(node->djeca[2], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska(node);
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska(node);
    }
}

void log_ili_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<log_i_izraz>"){
        log_i_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<log_ili_izraz>" 
    && node->djeca[1]->svojstva->znak == "OP_ILI" && node->djeca[2]->svojstva->znak == "<log_i_izraz>"){
        log_ili_izraz(node->djeca[0], tablica_node);
        log_i_izraz(node->djeca[2], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska(node);
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska(node);
    }
}

void izraz_pridruzivanja(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<log_ili_izraz>"){
        log_ili_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<postfiks_izraz>" 
    && node->djeca[1]->svojstva->znak == "OP_PRIDRUZI" && node->djeca[2]->svojstva->znak == "<izraz_pridruzivanja>"){
        izraz_pridruzivanja(node->djeca[2], tablica_node);

        string s = "\tMOVE R6, R" + to_string(registri);
	    kod.push_back(s);   
        s = "\tPUSH R" + to_string(registri);
        kod.push_back(s);
        brojPusheva++;

        neUzimajAdresu = true;
        postfiks_izraz(node->djeca[0], tablica_node);
        neUzimajAdresu = false;

        s = "\tPOP R" + to_string(registri);
        kod.push_back(s);
        brojPusheva--;

        if(node->djeca[0]->djeca.size() == 1){
            Tablica_Node* trenutna_tablica = tablica_node;

            s = "\tMOVE R" + to_string(registri) + ", R6";
            kod.push_back(s);
            
                int i = 0;
                while(trenutna_tablica != nullptr){
                    i += trenutna_tablica->adresa_na_stogu.size();
                    if(trenutna_tablica->roditelj == nullptr){
                        string s;
                        s = "\tSTORE R6, (" + adresa[node->djeca[0]->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka] + ")";
                        kod.push_back(s);

                        break;
                    }
                    if(trenutna_tablica->adresa_na_stogu.find(node->djeca[0]->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka) != trenutna_tablica->adresa_na_stogu.end()){
                        string s;
                        int pozicija = (i)*4 - trenutna_tablica->adresa_na_stogu[node->djeca[0]->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka] + brojPusheva*4;
                        //cout << trenutna_tablica->adresa_na_stogu[node->djeca[0]->svojstva->leks_jedinka] << endl;
                        if(pozicija < 0){
                            s = "\tSTORE R6, (R7" + pretvori_u_heksadekadski(pozicija) + ")";
                        }
                        else s = "\tSTORE R6, (R7+" + pretvori_u_heksadekadski(pozicija) + ")";
                        kod.push_back(s);

                        break;
                    }
                    trenutna_tablica = trenutna_tablica->roditelj;
                }
        }
        else if(node->djeca[0]->djeca.size() == 4 && node->djeca[0]->djeca[1]->svojstva->leks_jedinka == "["){
            Tablica_Node* trenutna_tablica = tablica_node;
            int i = 0;
                while(trenutna_tablica != nullptr){
                    i += trenutna_tablica->adresa_na_stogu.size();
                    string broj = node->djeca[0]->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka+"z0z";
                    if(trenutna_tablica->roditelj == nullptr){
                        s = "\tSHL R6, 2, R6";
                        kod.push_back(s);
                        s = "\tADD R6, " + adresa[broj] + ", R6";
                        kod.push_back(s);
                        s = "\tSTORE R" + to_string(registri) + ", (R6)";
                        kod.push_back(s);

                        break;
                    }
                    if(trenutna_tablica->adresa_na_stogu.find(node->djeca[0]->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka) != trenutna_tablica->adresa_na_stogu.end()){
                        string s;
                        s = "\tMOVE R" + to_string(registri) + ", R6";
                        kod.push_back(s);
                        int pozicija = (i)*4 - trenutna_tablica->adresa_na_stogu[node->djeca[0]->djeca[0]->djeca[0]->djeca[0]->svojstva->leks_jedinka] + brojPusheva*4;
                        //cout << trenutna_tablica->adresa_na_stogu[node->djeca[0]->svojstva->leks_jedinka] << endl;
                        if(pozicija < 0){
                            s = "\tSTORE R6, (R7" + pretvori_u_heksadekadski(pozicija) + ")";
                        }
                        else s = "\tSTORE R6, (R7+" + pretvori_u_heksadekadski(pozicija) + ")";
                        kod.push_back(s);

                        break;
                    }
                    trenutna_tablica = trenutna_tablica->roditelj;
                }
        }

        //cout << node->djeca[0]->svojstva->tip << " " << node->djeca[2]->svojstva->tip << endl;
        if(node->djeca[0]->svojstva->l_izraz == false || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, node->djeca[0]->svojstva->tip)){
            greska(node);
        }
        else{
            node->svojstva->tip = node->djeca[0]->svojstva->tip;
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska(node);
    }
}

void izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<izraz_pridruzivanja>"){
        izraz_pridruzivanja(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
        registri = 5;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<izraz>" 
    && node->djeca[1]->svojstva->znak == "ZAREZ" && node->djeca[2]->svojstva->znak == "<izraz_pridruzivanja>"){
        izraz(node->djeca[0], tablica_node);
        izraz_pridruzivanja(node->djeca[2], tablica_node);
        node->svojstva->tip = node->djeca[2]->svojstva->tip;
        node->svojstva->l_izraz = false;
        registri = 5;
    }

    else{
        greska(node);
    }
}

void slozena_naredba(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    Tablica_Node* nova_tablica = new Tablica_Node();
    nova_tablica->roditelj = tablica_node;
    tablica_node->djeca.push_back(nova_tablica);
    nova_tablica->main = tablica_node->main;
    isGlobal = false;

    int i = 0;
    if(node->roditelj->svojstva->znak == "<definicija_funkcije>"){
        if(node->roditelj->djeca[1]->svojstva->leks_jedinka == "main"){
            nova_tablica->main = true;
        }
        if(node->roditelj->djeca[3]->svojstva->znak == "<lista_parametara>"){
            for(; i < node->roditelj->djeca[3]->svojstva->argumenti.size(); i++){
                Node* parametar = new Node();
                parametar->svojstva->tip = node->roditelj->djeca[3]->svojstva->argumenti[i];
                parametar->svojstva->leks_jedinka = node->roditelj->djeca[3]->svojstva->argumenti_imena[i];
                parametar->svojstva->jeParametar = true;
                nova_tablica->zapis[parametar->svojstva->leks_jedinka] = parametar;
                nova_tablica->relativni_vrh += 4;
                nova_tablica->adresa_na_stogu[parametar->svojstva->leks_jedinka] = nova_tablica->relativni_vrh;
            }
        }
        nova_tablica->relativni_vrh += 4;
        nova_tablica->adresa_na_stogu["callback funkcije"] = nova_tablica->relativni_vrh ;
    }
    i+=1;

    if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "L_VIT_ZAGRADA" 
    && node->djeca[1]->svojstva->znak == "<lista_naredbi>" && node->djeca[2]->svojstva->znak == "D_VIT_ZAGRADA"){
        lista_naredbi(node->djeca[1], nova_tablica);
    }

    else if(node->djeca.size() == 4 && node->djeca[0]->svojstva->znak == "L_VIT_ZAGRADA" 
    && node->djeca[1]->svojstva->znak == "<lista_deklaracija>" && node->djeca[2]->svojstva->znak == "<lista_naredbi>"
    && node->djeca[3]->svojstva->znak == "D_VIT_ZAGRADA"){
        int j = 0;
        lista_deklaracija(node->djeca[1], nova_tablica);
        lista_naredbi(node->djeca[2], nova_tablica);
        
        if(!retSeDesio){
            for(auto node : nova_tablica->zapis){
                if(!node.second->svojstva->jeParametar){
                    j++;
                } 
            }
            string s = "\tADD R7, " + pretvori_u_heksadekadski(j*4) + ", R7";
            vrhStoga += i*4;
            kod.push_back(s);
        }
    }

    else{
        greska(node);
    }
    retSeDesio = false;
    if (tablica_node->roditelj == nullptr) { // Za definiranje globalnih varijabli #gl
            isGlobal = true; // Tako da primarni_izraz zna
        }
        else isGlobal = false;
}

void lista_naredbi(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<naredba>"){
        naredba(node->djeca[0], tablica_node);
    }

    else if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "<lista_naredbi>" 
    && node->djeca[1]->svojstva->znak == "<naredba>"){
        lista_naredbi(node->djeca[0], tablica_node);
        naredba(node->djeca[1], tablica_node);
    }

    else{
        greska(node);
    }
}

void naredba(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<slozena_naredba>"){
        slozena_naredba(node->djeca[0], tablica_node);
    }

    else if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<izraz_naredba>"){
        izraz_naredba(node->djeca[0], tablica_node);
    }

    else if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<naredba_grananja>"){
        naredba_grananja(node->djeca[0], tablica_node);
    }

    else if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<naredba_petlje>"){
        naredba_petlje(node->djeca[0], tablica_node);
    }

    else if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<naredba_skoka>"){
        naredba_skoka(node->djeca[0], tablica_node);
    }

    else{
        greska(node);
    }
}

void izraz_naredba(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "<izraz>" 
    && node->djeca[1]->svojstva->znak == "TOCKAZAREZ"){
        izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
    }

    else if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "TOCKAZAREZ"){
        node->svojstva->tip = "int";
    }

    else{
        greska(node);
    }
}

void naredba_grananja(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 5 && node->djeca[0]->svojstva->znak == "KR_IF" 
    && node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "<izraz>"
    && node->djeca[3]->svojstva->znak == "D_ZAGRADA" && node->djeca[4]->svojstva->znak == "<naredba>"){
        izraz(node->djeca[2], tablica_node);

        int i = elseLabel;
        elseLabel++;
        string s = "\tCMP R6, 0";
        kod.push_back(s);
        s = "\tJP_EQ END_IF" + to_string(i);
        kod.push_back(s);
        
        if(!moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska(node);
        }
        naredba(node->djeca[4], tablica_node);

        s = "END_IF" + to_string(i);
        kod.push_back(s);
    }

    else if(node->djeca.size() == 7 && node->djeca[0]->svojstva->znak == "KR_IF" 
    && node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "<izraz>"
    && node->djeca[3]->svojstva->znak == "D_ZAGRADA" && node->djeca[4]->svojstva->znak == "<naredba>"
    && node->djeca[5]->svojstva->znak == "KR_ELSE" && node->djeca[6]->svojstva->znak == "<naredba>"){
        izraz(node->djeca[2], tablica_node);

        int i = elseLabel;
        elseLabel++;
        string s = "\tCMP R6, 0";
        kod.push_back(s);
        s = "\tJP_EQ ELSE_" + to_string(i);
        kod.push_back(s);

        if(!moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska(node);
        }

        naredba(node->djeca[4], tablica_node);

        s = "\tJP END_IF_" + to_string(i);
        kod.push_back(s);
        s = "ELSE_" + to_string(i);
        kod.push_back(s);

        naredba(node->djeca[6], tablica_node);

        s = "END_IF_" + to_string(i);
        kod.push_back(s);
    }

    else{
        greska(node);
    }
}

void naredba_petlje(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 5 && node->djeca[0]->svojstva->znak == "KR_WHILE" 
    && node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "<izraz>"
    && node->djeca[3]->svojstva->znak == "D_ZAGRADA" && node->djeca[4]->svojstva->znak == "<naredba>"){
        string s;
        int i = elseLabel;
        elseLabel++;
        s = "WHILE_" + to_string(i);
        kod.push_back(s);

        izraz(node->djeca[2], tablica_node);
        
        s = "\tCMP R6, 0";
        kod.push_back(s);
        s = "\tJP_EQ END_WHILE_" + to_string(i);
        kod.push_back(s);

        if(!moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska(node);
        }
        naredba(node->djeca[4], tablica_node);
        s = "\tJP WHILE_" + to_string(i);
        kod.push_back(s);
        s = "END_WHILE_" + to_string(i);
        kod.push_back(s);

    }

    else if(node->djeca.size() == 6 && node->djeca[0]->svojstva->znak == "KR_FOR" 
    && node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "<izraz_naredba>"
    && node->djeca[3]->svojstva->znak == "<izraz_naredba>" && node->djeca[4]->svojstva->znak == "D_ZAGRADA"
    && node->djeca[5]->svojstva->znak == "<naredba>"){
        izraz_naredba(node->djeca[2], tablica_node);
        izraz_naredba(node->djeca[3], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[3]->svojstva->tip, "int")){
            greska(node);
        }
        naredba(node->djeca[5], tablica_node);
    }

    else if(node->djeca.size() == 7 && node->djeca[0]->svojstva->znak == "KR_FOR"
    && node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "<izraz_naredba>"
    && node->djeca[3]->svojstva->znak == "<izraz_naredba>" && node->djeca[4]->svojstva->znak == "<izraz>"
    && node->djeca[5]->svojstva->znak == "D_ZAGRADA" && node->djeca[6]->svojstva->znak == "<naredba>"){
        izraz_naredba(node->djeca[2], tablica_node);
        izraz_naredba(node->djeca[3], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[3]->svojstva->tip, "int")){
            greska(node);
        }
        izraz(node->djeca[4], tablica_node);
        naredba(node->djeca[6], tablica_node);
    }

    else{
        greska(node);
    }
}

void naredba_skoka(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);
    
    aktivnaNaredbaSkoka = true; //#ret

    if(node->djeca.size() == 2 && (node->djeca[0]->svojstva->znak == "KR_CONTINUE" 
    || node->djeca[0]->svojstva->znak == "KR_BREAK")
    && node->djeca[1]->svojstva->znak == "TOCKAZAREZ"){
        if(!jel_u_petlji(node)){
            greska(node);
        }
        node->svojstva->tip = "int";
    }

    else if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "KR_RETURN"
    && node->djeca[1]->svojstva->znak == "TOCKAZAREZ"){
        string tip = jel_u_funkciji(node);
        if(tip != "void"){
            greska(node);
        }
        node->svojstva->tip = "int";

        int j = 0;
        while(tablica_node->roditelj != nullptr){
            for(auto node : tablica_node->zapis){
                if(!node.second->svojstva->jeParametar){
                    j++;
                } 
            }
        tablica_node = tablica_node->roditelj;
        }
        kod.push_back("\tRET");
        retSeDesio = true;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "KR_RETURN"
    && node->djeca[1]->svojstva->znak == "<izraz>" && node->djeca[2]->svojstva->znak == "TOCKAZAREZ"){
        izraz(node->djeca[1], tablica_node);

        int j = 0;
        while(tablica_node->roditelj != nullptr){
            for(auto node : tablica_node->zapis){
                if(!node.second->svojstva->jeParametar){
                    j++;
                } 
            }
        tablica_node = tablica_node->roditelj;
        }
        
        string s = "\tADD R7, " + pretvori_u_heksadekadski(j*4) + ", R7";
        kod.push_back(s);

        kod.push_back("\tRET");
        retSeDesio = true;

        string tip = jel_u_funkciji(node);
        if(tip == "void" || !moze_se_pretvoriti(node->djeca[1]->svojstva->tip, tip)){
            greska(node);
        }
        node->svojstva->tip = tip;
    }

    else{
        greska(node);
    }
}

void prijevodna_jedinica(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "<prijevodna_jedinica>" 
    && node->djeca[1]->svojstva->znak == "<vanjska_deklaracija>"){
        prijevodna_jedinica(node->djeca[0], tablica_node);
        vanjska_deklaracija(node->djeca[1], tablica_node);
    }

    else if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<vanjska_deklaracija>"){
        vanjska_deklaracija(node->djeca[0], tablica_node);
    }

    else{
        greska(node);
    }
}

void vanjska_deklaracija(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<definicija_funkcije>"){
        definicija_funkcije(node->djeca[0], tablica_node);
    }

    else if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<deklaracija>"){
        deklaracija(node->djeca[0], tablica_node);
    }

    else{
        greska(node);
    }
}

void definicija_funkcije(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 6 && node->djeca[0]->svojstva->znak == "<ime_tipa>" 
    && node->djeca[1]->svojstva->znak == "IDN" && node->djeca[2]->svojstva->znak == "L_ZAGRADA"
    && node->djeca[3]->svojstva->znak == "KR_VOID" && node->djeca[4]->svojstva->znak == "D_ZAGRADA"
    && node->djeca[5]->svojstva->znak == "<slozena_naredba>"){
        if(node->djeca[1]->svojstva->leks_jedinka=="main") kod.push_back("\tRET");
        kod.push_back("F_" + node->djeca[1]->svojstva->leks_jedinka);
        funkcije[node->djeca[1]->svojstva->leks_jedinka] = node;
        ime_tipa(node->djeca[0], tablica_node);
        if(node->djeca[0]->svojstva->konst){
            greska(node);
        }
        if(tablica_node->roditelj != nullptr){ //mora biti globalna razina
            greska(node);
        }

        Node* prijasnja_definicija = provjeri_definicije(tablica_node, node->djeca[1]->svojstva->leks_jedinka);
        Node* prijasnja_deklaracija = provjeri_deklaracije(tablica_node, node->djeca[1]->svojstva->leks_jedinka);   
        if(prijasnja_definicija != nullptr){
            greska(node);
        }
        if(prijasnja_deklaracija != nullptr){
            if(prijasnja_deklaracija->svojstva->argumenti.size() != 0){
                greska(node);
            }
            if(prijasnja_deklaracija->svojstva->tip.find(" -> ") == string::npos 
            || split(prijasnja_deklaracija->svojstva->tip, " -> ")[1] != node->djeca[0]->svojstva->tip){
                greska(node);
            }
        }
        node->svojstva->tip = "funkcija() -> " + node->djeca[0]->svojstva->tip;
        node->svojstva->argumenti = {};
        node->svojstva->leks_jedinka = node->djeca[1]->svojstva->leks_jedinka;
        tablica_node->zapis.insert({node->djeca[1]->svojstva->leks_jedinka, node});
        tablica_node->definirane_funkcije.push_back(node);  
        slozena_naredba(node->djeca[5], tablica_node);
        node->svojstva->fja_definirana = true;
    }

    else if(node->djeca.size() == 6 && node->djeca[0]->svojstva->znak == "<ime_tipa>"
    && node->djeca[1]->svojstva->znak == "IDN" && node->djeca[2]->svojstva->znak == "L_ZAGRADA"
    && node->djeca[3]->svojstva->znak == "<lista_parametara>" && node->djeca[4]->svojstva->znak == "D_ZAGRADA"
    && node->djeca[5]->svojstva->znak == "<slozena_naredba>"){
        kod.push_back("F_" + node->djeca[1]->svojstva->leks_jedinka);
        funkcije[node->djeca[1]->svojstva->leks_jedinka] = node;
        ime_tipa(node->djeca[0], tablica_node);
        if(node->djeca[0]->svojstva->konst){
            greska(node);
        }

        lista_parametara(node->djeca[3], tablica_node);
        Node* prijasnja_definicija = provjeri_tablicu(node->djeca[1]->svojstva->leks_jedinka, tablica_node);
        if(prijasnja_definicija != nullptr){
            if(prijasnja_definicija->svojstva->argumenti.size() != node->djeca[3]->svojstva->argumenti.size()){
                greska(node);
            }
            for(int i = 0; i < node->djeca[3]->svojstva->argumenti.size(); i++){
                if(prijasnja_definicija->svojstva->argumenti[i] != node->djeca[3]->svojstva->argumenti[i]){
                    greska(node);
                }
            }
            if(prijasnja_definicija->svojstva->tip.find(" -> ") == string::npos 
            || split(prijasnja_definicija->svojstva->tip, " -> ")[1] != node->djeca[0]->svojstva->tip){
                greska(node);
            }
        }

        node->svojstva->tip = "funkcija(";
        for(int i = 0; i < node->djeca[3]->svojstva->argumenti.size(); i++){
            node->svojstva->tip += node->djeca[3]->svojstva->argumenti[i];
            if(i != node->djeca[3]->svojstva->argumenti.size() - 1){
                node->svojstva->tip += ", ";
            }
        }
        node->svojstva->tip += ") -> " + node->djeca[0]->svojstva->tip;

        node->svojstva->argumenti = node->djeca[3]->svojstva->argumenti;
        node->svojstva->leks_jedinka = node->djeca[1]->svojstva->leks_jedinka;
        tablica_node->zapis.insert({node->djeca[1]->svojstva->leks_jedinka, node});
        tablica_node->definirane_funkcije.push_back(node); 
        slozena_naredba(node->djeca[5], tablica_node);
        node->svojstva->fja_definirana = true;
    }
    else{
        greska(node);
    }
}

void lista_parametara(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<deklaracija_parametra>"){
        deklaracija_parametra(node->djeca[0], tablica_node);
        node->svojstva->argumenti.push_back(node->djeca[0]->svojstva->tip);
        node->svojstva->argumenti_imena.push_back(node->djeca[0]->svojstva->leks_jedinka);
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<lista_parametara>" 
    && node->djeca[1]->svojstva->znak == "ZAREZ" && node->djeca[2]->svojstva->znak == "<deklaracija_parametra>"){
        lista_parametara(node->djeca[0], tablica_node);
        deklaracija_parametra(node->djeca[2], tablica_node);
        for(int i = 0; i < node->djeca[0]->svojstva->argumenti_imena.size(); i++){
            if(node->djeca[0]->svojstva->argumenti_imena[i] == node->djeca[2]->svojstva->leks_jedinka){
                greska(node);
            }
        }
        node->svojstva->argumenti = node->djeca[0]->svojstva->argumenti;
        node->svojstva->argumenti.push_back(node->djeca[2]->svojstva->tip);
        node->svojstva->argumenti_imena = node->djeca[0]->svojstva->argumenti_imena;
        node->svojstva->argumenti_imena.push_back(node->djeca[2]->svojstva->leks_jedinka);
    }

    else{
        greska(node);
    }
}

void deklaracija_parametra(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "<ime_tipa>" 
    && node->djeca[1]->svojstva->znak == "IDN"){
        ime_tipa(node->djeca[0], tablica_node);
        if(node->djeca[0]->svojstva->tip == "void"){
            greska(node);
        }
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->leks_jedinka = node->djeca[1]->svojstva->leks_jedinka;
    }

    else if(node->djeca.size() == 4 && node->djeca[0]->svojstva->znak == "<ime_tipa>"
    && node->djeca[1]->svojstva->znak == "IDN" && node->djeca[2]->svojstva->znak == "L_UGL_ZAGRADA"
    && node->djeca[3]->svojstva->znak == "D_UGL_ZAGRADA"){
        ime_tipa(node->djeca[0], tablica_node);
        if(node->djeca[0]->svojstva->tip == "void"){
            greska(node);
        }
        node->svojstva->tip = "niz(" + node->djeca[0]->svojstva->tip + ")";
        node->svojstva->leks_jedinka = node->djeca[1]->svojstva->leks_jedinka;
    }

    else{
        greska(node);
    }
}

void lista_deklaracija(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<deklaracija>"){
        deklaracija(node->djeca[0], tablica_node);
    }

    else if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "<lista_deklaracija>" 
    && node->djeca[1]->svojstva->znak == "<deklaracija>"){
        lista_deklaracija(node->djeca[0], tablica_node);
        deklaracija(node->djeca[1], tablica_node);
    }

    else{
        greska(node);
    }
}

void deklaracija(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<ime_tipa>" 
    && node->djeca[1]->svojstva->znak == "<lista_init_deklaratora>" && node->djeca[2]->svojstva->znak == "TOCKAZAREZ"){
        ime_tipa(node->djeca[0], tablica_node);
        node->djeca[1]->svojstva->ntip = node->djeca[0]->svojstva->tip;
        // -----------------------------------------------------------------------
        aktivnaDeklaracija = true;
        
        // -----------------------------------------------------------------------        
        lista_init_deklaratora(node->djeca[1], tablica_node);
        aktivnaDeklaracija = false;
    }

    else{
        greska(node);
    }
}

void lista_init_deklaratora(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<init_deklarator>"){
        node->djeca[0]->svojstva->ntip = node->svojstva->ntip;
        init_deklarator(node->djeca[0], tablica_node);
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<lista_init_deklaratora>" 
    && node->djeca[1]->svojstva->znak == "ZAREZ" && node->djeca[2]->svojstva->znak == "<init_deklarator>"){
        node->djeca[0]->svojstva->ntip = node->svojstva->ntip;
        node->djeca[2]->svojstva->ntip = node->svojstva->ntip;
        lista_init_deklaratora(node->djeca[0], tablica_node);
        init_deklarator(node->djeca[2], tablica_node);
    }

    else{
        greska(node);
    }
}

void init_deklarator(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    int i;
    if(tablica_node->adresa_na_stogu.find("callback funkcije") == tablica_node->adresa_na_stogu.end()){
        i = 1;
    }
    else i = tablica_node->relativni_vrh+4;
    string s;

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<izravni_deklarator>"){
        node->djeca[0]->svojstva->ntip = node->svojstva->ntip;
        izravni_deklarator(node->djeca[0], tablica_node);

        if(node->djeca[0]->djeca.size() == 1 && node->djeca[0]->djeca[0]->svojstva->znak == "IDN"){
            if(!isGlobal){
                s = "\tSUB R7, 4, R7";
                kod.push_back(s);
                tablica_node->relativni_vrh += 4;
                tablica_node->adresa_na_stogu[node->djeca[0]->djeca[0]->svojstva->leks_jedinka] = tablica_node->relativni_vrh;
                ++i;
            }
            else{
                string identifikator = node->djeca[0]->djeca[0]->svojstva->leks_jedinka;
                string s = "G_" + identifikator + "\tDW 0";
                kod.push_back(s);     
                adresa[identifikator] = "G_" + identifikator; //#ret
            }
        }
        else if(node->djeca[0]->djeca.size() == 4 && node->djeca[0]->djeca[0]->svojstva->znak == "IDN" 
        && node->djeca[0]->djeca[1]->svojstva->znak == "L_UGL_ZAGRADA" && node->djeca[0]->djeca[2]->svojstva->znak == "BROJ"
        && node->djeca[0]->djeca[3]->svojstva->znak == "D_UGL_ZAGRADA"){
            trenutniArray = node->djeca[0]->djeca[0]->svojstva->leks_jedinka;
            if(!isGlobal){
                for(int i = 0; i < stoi(node->djeca[0]->djeca[2]->svojstva->leks_jedinka); i++){
                    s = "\tSUB R7, 4, R7";
                    kod.push_back(s);
                    tablica_node->relativni_vrh += 4;
                    tablica_node->adresa_na_stogu[trenutniArray+"z"+to_string(i)+"z"] = tablica_node->relativni_vrh;
                }
            }
            else{
                for(int i = 0; i < stoi(node->djeca[0]->djeca[2]->svojstva->leks_jedinka); i++){
                    string identifikator = trenutniArray + "z" + to_string(brojPushevaArray) + "z";
                    string s = "G_" + identifikator + "\tDW 0";
                    kod.push_back(s);     
                    adresa[identifikator] = "G_" + identifikator; //#ret
                    brojPushevaArray += 4;
                } 
            }
        }

        if(node->djeca[0]->svojstva->konst){
            greska(node);
        }
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<izravni_deklarator>" 
    && node->djeca[1]->svojstva->znak == "OP_PRIDRUZI" && node->djeca[2]->svojstva->znak == "<inicijalizator>"){
        
        node->djeca[0]->svojstva->ntip = node->svojstva->ntip;
        inicijalizator(node->djeca[2], tablica_node);
        if(node->djeca[0]->djeca.size() == 1 && node->djeca[0]->djeca[0]->svojstva->znak == "IDN"){
            if(!isGlobal){
                s = "\tPUSH R6";
                kod.push_back(s);
                tablica_node->relativni_vrh += 4;
                tablica_node->adresa_na_stogu[node->djeca[0]->djeca[0]->svojstva->leks_jedinka] = tablica_node->relativni_vrh;
                //cout << node->djeca[0]->djeca[0]->svojstva->leks_jedinka << " " << tablica_node->relativni_vrh << endl;
                ++i;
            }
            else{
                string identifikator = node->djeca[0]->djeca[0]->svojstva->leks_jedinka;
                string s = "G_" + identifikator;
                kod.push_back(s);     
                adresa[identifikator] = "G_" + identifikator; //#ret
                s = "\tSTORE R6, (" + adresa[identifikator] + ")";
                kod.push_back(s);
            }
            
        }
        else if(node->djeca[0]->djeca.size() == 4 && node->djeca[0]->djeca[0]->svojstva->znak == "IDN" 
        && node->djeca[0]->djeca[1]->svojstva->znak == "L_UGL_ZAGRADA" && node->djeca[0]->djeca[2]->svojstva->znak == "BROJ"
        && node->djeca[0]->djeca[3]->svojstva->znak == "D_UGL_ZAGRADA"){
            trenutniArray = node->djeca[0]->djeca[0]->svojstva->leks_jedinka;
            if(!isGlobal){
                for(int i = 0; i < stoi(node->djeca[0]->djeca[2]->svojstva->leks_jedinka); i++){
                    s = "\tSUB R7, 4, R7";
                    kod.push_back(s);
                    tablica_node->relativni_vrh += 4;
                    tablica_node->adresa_na_stogu[trenutniArray+"z"+to_string(i)+"z"] = tablica_node->relativni_vrh;
                }
            }
            else{
                for(int i = 0; i < stoi(node->djeca[0]->djeca[2]->svojstva->leks_jedinka); i++){
                    string identifikator = trenutniArray + "z" + to_string(i*4) + "z";
                    string s = "G_" + identifikator + "\tDW 0";
                    kod.push_back(s);
                    adresa[identifikator] = "G_" + identifikator; //#ret
                    brojPushevaArray += 4;
                }
                for(int i = stoi(node->djeca[0]->djeca[2]->svojstva->leks_jedinka); i >= 0; i--){
                    string identifikator = trenutniArray + "z" + to_string(i*4) + "z";
                    string s;
                    if(brojPusheva==i+1){
                        s = "\tPOP R6";
                        kod.push_back(s);
                        brojPusheva--;
                        s = "\tSTORE R6, (" + adresa[identifikator] + ")";
                        kod.push_back(s);
                    }
                } 
            }
        }
        izravni_deklarator(node->djeca[0], tablica_node);
        
        brojPushevaArray = 0;

        

        Node* provjera = jel_ide_u_identifikator(node->djeca[2]);
        if(provjera != nullptr && provjera->svojstva->tip.find("niz(") != string::npos){
            if(!provjera->svojstva->konst){
                greska(node);
            }
        }

        string tip_deklrator = node->djeca[0]->svojstva->tip;
        string tip_inicijalizator = node->djeca[2]->svojstva->tip;
        if (tip_deklrator.substr(0, 4) != "niz(") {

            if (!moze_se_pretvoriti(tip_inicijalizator, tip_deklrator)) {
                
                greska(node);
            }
        } else if (tip_deklrator.substr(0, 4) == "niz(") {

            string element_tip = tip_deklrator.substr(4, tip_deklrator.size() - 5);
            if (node->djeca[2]->svojstva->broj_elemenata > node->djeca[0]->svojstva->broj_elemenata) {

                greska(node);
            }
            for (const string& u : node->djeca[2]->svojstva->tipovi) {
                if (!moze_se_pretvoriti(u, element_tip)) {
                    greska(node);
                }
            }
        } else {
            greska(node);
        }
    }

    else{
        greska(node);
    }
    trenutniArray = "";
}

void izravni_deklarator(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    trenutniIzravniDeklarator = node; //#gl

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "IDN"){
        node->svojstva->tip = node->svojstva->ntip;
        node->svojstva->leks_jedinka = node->djeca[0]->svojstva->leks_jedinka;
        if(node->svojstva->tip.substr(0, 5) == "const"){
            node->svojstva->konst = true;
        }

        if(node->svojstva->tip == "void"){
            greska(node);
        }
        if(provjeri_tablicu_lokalno(node->djeca[0]->svojstva->leks_jedinka, tablica_node) != nullptr){
            greska(node);
        }

        tablica_node->zapis.insert({node->djeca[0]->svojstva->leks_jedinka, node});
    }

    else if(node->djeca.size() == 4 && node->djeca[0]->svojstva->znak == "IDN" 
    && node->djeca[1]->svojstva->znak == "L_UGL_ZAGRADA" && node->djeca[2]->svojstva->znak == "BROJ"
    && node->djeca[3]->svojstva->znak == "D_UGL_ZAGRADA"){
        //cout << node->djeca[2]->svojstva->leks_jedinka << "a a" << node->djeca[2]->svojstva->tip << endl;
        if(node->djeca[2]->svojstva->leks_jedinka.size() > 5 || stoi(node->djeca[2]->svojstva->leks_jedinka) <= 0
        || stoi(node->djeca[2]->svojstva->leks_jedinka) > 1024){
            greska(node);
        }
        if(provjeri_tablicu_lokalno(node->djeca[0]->svojstva->leks_jedinka, tablica_node) != nullptr){
            greska(node);
        }
        if(node->svojstva->ntip == "void"){
            greska(node);
        }

        node->svojstva->tip = "niz(" + node->svojstva->ntip + ")";
        node->svojstva->leks_jedinka = node->djeca[0]->svojstva->leks_jedinka + "z" + node->djeca[2]->svojstva->leks_jedinka + "z";
        if(node->svojstva->ntip.substr(0, 5) == "const"){
            node->svojstva->konst = true;
        }
        node->svojstva->broj_elemenata = stoi(node->djeca[2]->svojstva->leks_jedinka);

        tablica_node->zapis.insert({node->djeca[0]->svojstva->leks_jedinka, node});
    }

    else if(node->djeca.size() == 4 && node->djeca[0]->svojstva->znak == "IDN"
    && node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "KR_VOID"
    && node->djeca[3]->svojstva->znak == "D_ZAGRADA"){
        Node* prijasnja_definicija = provjeri_definicije(tablica_node, node->djeca[0]->svojstva->leks_jedinka);
        Node* prijasnja_deklaracija = provjeri_deklaracije(tablica_node, node->djeca[0]->svojstva->leks_jedinka);   
        if(prijasnja_definicija != nullptr){
            greska(node);
        }
        if(prijasnja_deklaracija != nullptr){
            if(prijasnja_deklaracija->svojstva->argumenti.size() != 0){
                greska(node);
            }
            if(prijasnja_deklaracija->svojstva->tip.find(" -> ") == string::npos
            || split(prijasnja_deklaracija->svojstva->tip, " -> ")[1] != node->svojstva->ntip
            || prijasnja_deklaracija->svojstva->tip.find("funkcija(") == string::npos){
                greska(node);
            }
        }

        node->svojstva->tip = "funkcija() -> " + node->svojstva->ntip;
        node->svojstva->leks_jedinka = node->djeca[0]->svojstva->leks_jedinka;

        tablica_node->zapis.insert({node->svojstva->leks_jedinka, node});
        tablica_node->deklarirane_funkcije.push_back(node);
    }

    else if(node->djeca.size() == 4 && node->djeca[0]->svojstva->znak == "IDN"
    && node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "<lista_parametara>"
    && node->djeca[3]->svojstva->znak == "D_ZAGRADA"){
        lista_parametara(node->djeca[2], tablica_node);
        Node* prijasnja_definicija = provjeri_definicije(tablica_node, node->djeca[0]->svojstva->leks_jedinka);
        Node* prijasnja_deklaracija = provjeri_deklaracije(tablica_node, node->djeca[0]->svojstva->leks_jedinka);   
        if(prijasnja_definicija != nullptr){
            greska(node);
        }
        if(prijasnja_deklaracija != nullptr){
            if(prijasnja_deklaracija->svojstva->argumenti.size() != node->djeca[2]->svojstva->argumenti.size()){
                greska(node);
            }
            for(int i = 0; i < node->djeca[2]->svojstva->argumenti.size(); i++){
                if(prijasnja_deklaracija->svojstva->argumenti[i] != node->djeca[2]->svojstva->argumenti[i]){
                    greska(node);
                }
            }
            if(prijasnja_deklaracija->svojstva->tip.find(" -> ") == string::npos
            || split(prijasnja_deklaracija->svojstva->tip, " -> ")[1] != node->svojstva->ntip
            || prijasnja_deklaracija->svojstva->tip.find("funkcija(") == string::npos){
                greska(node);
            }
        }

        node->svojstva->tip = "funkcija(";
        for(int i = 0; i < node->djeca[2]->svojstva->argumenti.size(); i++){
            node->svojstva->tip += node->djeca[2]->svojstva->argumenti[i];
            if(i < node->djeca[2]->svojstva->argumenti.size() - 1){
                node->svojstva->tip += ", ";
            }
        }
        node->svojstva->tip += ") -> " + node->svojstva->ntip;
        node->svojstva->leks_jedinka = node->djeca[0]->svojstva->leks_jedinka;
        node->svojstva->argumenti = node->djeca[2]->svojstva->argumenti;

        tablica_node->zapis.insert({node->svojstva->leks_jedinka, node});
        tablica_node->deklarirane_funkcije.push_back(node);
    }
    
    else{
        greska(node);
    }
}

void inicijalizator(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<izraz_pridruzivanja>"){
        izraz_pridruzivanja(node->djeca[0], tablica_node);
        int broj = jel_ide_u_niz_znakova(node);
        
        if(broj != 0){
            node->svojstva->tip = "NIZ_ZNAKOVA";
            node->svojstva->broj_elemenata = broj;
            for(int i = 0; i < broj; i++){
                node->svojstva->tipovi.push_back("char");
            }
        }
        else{
            node->svojstva->tip = node->djeca[0]->svojstva->tip;
        }
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "L_VIT_ZAGRADA" 
    && node->djeca[1]->svojstva->znak == "<lista_izraza_pridruzivanja>" && node->djeca[2]->svojstva->znak == "D_VIT_ZAGRADA"){
        lista_izraza_pridruzivanja(node->djeca[1], tablica_node);
        node->svojstva->tip = "niz(";
        for(int i = 0; i < node->djeca[1]->svojstva->tipovi.size(); i++){
            node->svojstva->tip += node->djeca[1]->svojstva->tipovi[i];
            if(i != node->djeca[1]->svojstva->tipovi.size() - 1){
                node->svojstva->tip += ", ";
            }
        }
        node->svojstva->tip += ")";
        node->svojstva->tipovi = node->djeca[1]->svojstva->tipovi;
        node->svojstva->broj_elemenata = node->djeca[1]->svojstva->broj_elemenata;
    }

    else{
        greska(node);
    }
}

void lista_izraza_pridruzivanja(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska(node);

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<izraz_pridruzivanja>"){
        izraz_pridruzivanja(node->djeca[0], tablica_node);
        node->svojstva->tipovi.push_back(node->djeca[0]->svojstva->tip);
        node->svojstva->broj_elemenata = 1;

        if(!isGlobal){
            string s;
            s = "\tSTORE R6, (R7 + " + to_string(tablica_node->adresa_na_stogu[trenutniArray]+brojPushevaArray) + ")";
            kod.push_back(s);
            brojPushevaArray+=4;
        }
        else{
            string s;
            s = "\tPUSH R6";
            brojPusheva++;
            kod.push_back(s);
        }
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<lista_izraza_pridruzivanja>" 
    && node->djeca[1]->svojstva->znak == "ZAREZ" && node->djeca[2]->svojstva->znak == "<izraz_pridruzivanja>"){
        lista_izraza_pridruzivanja(node->djeca[0], tablica_node);
        izraz_pridruzivanja(node->djeca[2], tablica_node);
        node->svojstva->tipovi = node->djeca[0]->svojstva->tipovi;
        node->svojstva->tipovi.push_back(node->djeca[2]->svojstva->tip);
        node->svojstva->broj_elemenata = node->djeca[0]->svojstva->broj_elemenata + 1;

        if(!isGlobal){
            string s;
            s = "\tSTORE R6, (R7 + " + to_string(tablica_node->adresa_na_stogu[trenutniArray]+brojPushevaArray) + ")";
            kod.push_back(s);
            brojPushevaArray+=4;
        }
        else{
            string s;
            s = "\tPUSH R6";
            brojPusheva++;
            kod.push_back(s);
        }
    }

    else{
        greska(node);
    }
}
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

Node* parsiraj(string filename) {
    ifstream file(filename);
    string line;
    vector<Node*> trenutni_roditelji = {};
    Node* root = nullptr;


    while (getline(file, line)) {
        int level = 0;
        while (line[level] == ' ') {
            if(line.size() == level) greska(root);
            level++;
        }

        line = line.substr(level);

        Node* node = new Node();
        if(root == nullptr){
            root = node;
        }
        if(line[0] == '<'){
            node->svojstva->znak = line;
        }
        else{
            vector<string> dijelovi = split(line, " ");
            node->svojstva->znak = dijelovi[0];
            node->svojstva->redak = stoi(dijelovi[1]);	
            node->svojstva->leks_jedinka = dijelovi[2];
        }


        if(level > 0){
            trenutni_roditelji[level-1]->djeca.push_back(node);
            node->roditelj = trenutni_roditelji[level-1];
        }
        if(trenutni_roditelji.size() == level){
            trenutni_roditelji.push_back(node);
        }
        else if(trenutni_roditelji.size() > level){
            trenutni_roditelji[level] = node;
        }      
        else{
            greska(root);
        }
    }
    return root;
}

void provjera_main_funkcije(Tablica_Node* tablica_node){
    Tablica_Node* trenutna_tablica = tablica_node;
    while(trenutna_tablica != nullptr){
        if(trenutna_tablica->zapis.find("main") != trenutna_tablica->zapis.end()){
            Node* main_funkcija = trenutna_tablica->zapis["main"];
            if(main_funkcija->svojstva->tip == "funkcija() -> int" 
            && main_funkcija->svojstva->argumenti.size() == 0){
                return;
            }
        }
        trenutna_tablica = trenutna_tablica->roditelj;
    }
    cout << "main" << endl;
}

void provjeri_definirane_funkcije(Tablica_Node* tablica, Tablica_Node* tablica_nova) {
    Tablica_Node* trenutna_tablica = tablica_nova;

    for(auto it = trenutna_tablica->deklarirane_funkcije.begin(); it != trenutna_tablica->deklarirane_funkcije.end(); it++){
        bool uspio = false;
        for(auto it2 = tablica->definirane_funkcije.begin(); it2 != tablica->definirane_funkcije.end(); it2++){
            if((*it)->svojstva->leks_jedinka == (*it2)->svojstva->leks_jedinka
            && (*it)->svojstva->tip == (*it2)->svojstva->tip){
                uspio = true;
                break;
            }
        }
        if(!uspio){
            cout << "funkcija" << endl;
            return;
        }
    }

    for(auto dijete : trenutna_tablica->djeca){
        provjeri_definirane_funkcije(tablica, dijete);
    }
}

void ispisi_kod(string filename){
    ofstream file(filename);
    for(auto it = kod.begin(); it != kod.end(); it++){
        file << *it << endl;
    }
}

int main(void){
    Node* root = nullptr;
    Tablica_Node* tablica_node = new Tablica_Node(nullptr);

    kod.push_back("\tMOVE 40000, R7");
    kod.push_back("\tCALL globalne");
    kod.push_back("\tCALL F_main");
    kod.push_back("\tHALT");
    
    ispisi_div_funkciju();
    kod.push_back("globalne");

    root = parsiraj("ulaz.txt");
    prijevodna_jedinica(root, tablica_node);

    ispisi_kod("Izlaz.txt");
    return 0;
}

            



