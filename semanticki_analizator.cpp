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

using namespace std;

class Node_svojstva{
    public:
        string znak = ""; //uniformni znak(tipa IDN, BROJ...)
        int redak = -1;
        string leks_jedinka = ""; //origigi sto pise (a, 12, +...)
        bool konst;
        string tip = "";
        string ntip = ""; //nasljedni tip bitan za provjeru tocnosti deklaracija
        bool l_izraz = "";
        vector<string> argumenti = {};
        vector<string> argumenti_imena = {};

        Node_svojstva(string znak, int redak, string leks_jedinka) : znak(znak), redak(redak), leks_jedinka(leks_jedinka), konst(false) {}

        Node_svojstva() = default;

        Node_svojstva(string znak, int redak, string leks_jedinka, bool konst) : znak(znak), redak(redak), leks_jedinka(leks_jedinka), konst(konst) {}

};

class Node{
    public:
        Node_svojstva* svojstva = nullptr;
        Node* roditelj = nullptr;
        vector<Node*> djeca;

        Node(Node_svojstva* svojstva) : svojstva(svojstva){};

        Node() = default;

};

class Tablica_Node{
    public:
        map<string, Node*> zapis;
        Tablica_Node* roditelj = nullptr;

        Tablica_Node(Tablica_Node* roditelj = nullptr) : roditelj(roditelj) {}
};

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

void greska(){ //napraviti funkciju za ispis greske
    cout << "0" << endl;
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
        } else {
            uspio = false;
            break;
        }
    }
    return uspio;
}

bool moze_se_pretvoriti(string prvi, string drugi) { //provjerava moze li se tip from pretvoriti u tip to
    if (prvi == drugi) return true;
    if (prvi == "char" && drugi == "int") return true;
    if (prvi == "const(char)" && (drugi == "char" || drugi == "int" || drugi == "const(int)")) return true;
    if (prvi == "const(int)" && drugi == "int") return true;
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


//------------------------------------------------------------------------------------------------
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
    if(node->svojstva == nullptr) greska();
    if(node->djeca.size() == 1){
        if(node->djeca[0]->svojstva->znak == "IDN"){ //ako je dijete IDN
            if(provjeri_tablicu(node->djeca[0]->svojstva->leks_jedinka, tablica_node) == nullptr){ //ako ne postoji u tablici
                greska();
            }
            else{ //ako postoji u tablici, postavi svojstva
                naslijedi_svojstva(node, tablica_node->zapis[node->djeca[0]->svojstva->leks_jedinka]);
            }
        }
        else if(node->djeca[0]->svojstva->znak == "BROJ"){ //ako je dijete BROJ
            long long broj = stoll(node->svojstva->leks_jedinka);
            if(!(broj <= INT_MAX && broj >= INT_MIN)){
                greska();
            }
            else{ //ako je u intervalu int, postavi svojstva
                node->svojstva->tip = "int";
                node->svojstva->l_izraz = false;
            }
        }
        else if(node->djeca[0]->svojstva->znak == "ZNAK"){ //ako je dijete ZNAK
            string leks_jedinka = node->svojstva->leks_jedinka;
            if(provjeri_znak(leks_jedinka)){ //ako je ispravan znak, postavi svojstva
                node->svojstva->tip = "char";
                node->svojstva->l_izraz = false;
            } else {
                greska();
            }
        }
        else if(node->djeca[0]->svojstva->znak == "NIZ_ZNAKOVA"){ //ako je dijete NIZ_ZNAKOVA
            string leks_jedinka = node->svojstva->leks_jedinka;
            if(provjeri_niz_znakova(leks_jedinka)){ //ako je ispravan niz znakova, postavi svojstva
                node->svojstva->tip = "niz(const(char))";
                node->svojstva->l_izraz = false;
            } else {
                greska();
            }
        }
        else{
            greska();
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
        greska();
        
    }
}

void postfiks_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    //ako je dijete <primarni_izraz>
    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<primarni_izraz>"){
        primarni_izraz(node->djeca[0], tablica_node); //provjeri primarni izraz
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 4 && node->djeca[0]->svojstva->znak == "<postfiks_izraz>" 
    && node->djeca[1]->svojstva->znak == "L_UGL_ZAGRADA" && node->djeca[2]->svojstva->znak == "<izraz>"
    && node->djeca[3]->svojstva->znak == "D_UGL_ZAGRADA"){
        postfiks_izraz(node->djeca[0], tablica_node);
        if(node->djeca[0]->svojstva->tip.substr(0, 3) != "niz"){
            greska();
        }
        else{ //ako se radi o nizu
            string podtip = node->djeca[0]->svojstva->tip.substr(4, node->djeca[0]->svojstva->tip.size()-5);
            if(podtip.substr(0, 5) == "const"){
                node->svojstva->tip = "char";
                node->svojstva->l_izraz = false;
            }
            else{
                node->svojstva->tip = "char";
                node->svojstva->l_izraz = true;
            }
        }

        izraz(node->djeca[2], tablica_node);
        if (!moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")) {
            greska();
        }
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<postfiks_izraz>" 
    && node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "D_ZAGRADA"){
        postfiks_izraz(node->djeca[0], tablica_node);
        if(node->djeca[0]->svojstva->tip.substr(0, 8) != "funkcija"){
            greska();
        }
        else{
            if(!node->djeca[0]->svojstva->argumenti.empty()){
                greska();
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
        if(node->djeca[0]->svojstva->tip.substr(0, 8) != "funkcija"){
            greska();
        }
        else{
            if(node->djeca[0]->svojstva->argumenti.empty()){
                greska();
            }
            else{
                if(node->djeca[0]->svojstva->argumenti.size() != node->djeca[2]->djeca.size()){
                    greska();
                }
                for(int i = 0; i < node->djeca[0]->svojstva->argumenti.size(); i++){
                    if(!moze_se_pretvoriti(node->djeca[2]->djeca[i]->svojstva->tip, node->djeca[0]->svojstva->argumenti[i])){
                        greska();
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
        if(node->djeca[0]->svojstva->l_izraz == false || !moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int")){
            greska();
        }
        node->svojstva->tip = "int";
        node->svojstva->l_izraz = false;
    }

    else{
        greska();
    }
}

void lista_argumenata(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();
    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<izraz_pridruzivanja>"){
        izraz_pridruzivanja(node->djeca[0], tablica_node);
        node->svojstva->argumenti.push_back(node->djeca[0]->svojstva->tip);
    }
    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<lista_argumenata>"
    && node->djeca[1]->svojstva->znak == "ZAREZ" && node->djeca[2]->svojstva->znak == "<izraz_pridruzivanja>"){
        lista_argumenata(node->djeca[0], tablica_node);
        izraz_pridruzivanja(node->djeca[2], tablica_node);
        node->svojstva->argumenti = node->djeca[0]->svojstva->argumenti;
        node->svojstva->argumenti.push_back(node->djeca[2]->svojstva->tip);
    }
    else{
        greska();
    }
}

void unarni_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<postfiks_izraz>"){
        postfiks_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 2 && (node->djeca[0]->svojstva->znak == "OP_INC" ||node->djeca[0]->svojstva->znak == "OP_DEC") 
    && node->djeca[1]->svojstva->znak == "<unarni_izraz>"){
        unarni_izraz(node->djeca[1], tablica_node);
        if(node->djeca[1]->svojstva->l_izraz == false || !moze_se_pretvoriti(node->djeca[1]->svojstva->tip, "int")){
            greska();
        }
        node->svojstva->tip = "int";
        node->svojstva->l_izraz = false;
    }

    else if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "<unarani_operator>"
    && node->djeca[1]->svojstva->znak == "<cast_izraz"){
        cast_izraz(node->djeca[1], tablica_node);
        if(moze_se_pretvoriti(node->djeca[1]->svojstva->tip, "int")){
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
        else{
            greska();
        }
    }
}

void unarni_operator(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && (node->djeca[0]->svojstva->znak == "PLUS" || node->djeca[0]->svojstva->znak == "MINUS"
    || node->djeca[0]->svojstva->znak == "OP_TILDA" || node->djeca[0]->svojstva->znak == "OP_NEG")){
        node->svojstva->tip = "int"; //mozda nije potrebno
        node->svojstva->l_izraz = false;
    }

    else{
        greska();
    }
}

void cast_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

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
        if(moze_se_pretvoriti(node->djeca[3]->svojstva->tip, node->djeca[1]->svojstva->tip)){
            node->svojstva->tip = node->djeca[1]->svojstva->tip;
            node->svojstva->l_izraz = false;
        }
        else{
            greska();
        }
    }

    else{
        greska();
    }
}

void ime_tipa(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<specifikator_tipa>"){
        specifikator_tipa(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
    }

    else if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "KR_CONST" 
    && node->djeca[1]->svojstva->znak == "<specifikator_tipa>"){
        specifikator_tipa(node->djeca[1], tablica_node);
        if(node->djeca[1]->svojstva->tip == "void"){
            greska();
        }
        node->svojstva->tip = "const(" + node->djeca[1]->svojstva->tip + ")";
        node->svojstva->konst = true;
    }

    else{
        greska();
    }
}

void specifikator_tipa(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

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
        greska();
    }
}

void multiplikativni_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

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
            greska();
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska();
    }
}

void aditivni_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<multiplikativni_izraz>"){
        multiplikativni_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<aditivni_izraz>" 
    && (node->djeca[1]->svojstva->znak == "PLUS" || node->djeca[1]->svojstva->znak == "MINUS")
    && node->djeca[2]->svojstva->znak == "<multiplikativni_izraz>"){
        aditivni_izraz(node->djeca[0], tablica_node);
        multiplikativni_izraz(node->djeca[2], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska();
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska();
    }
}

void odnosni_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<aditivni_izraz>"){
        aditivni_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<odnosni_izraz>" 
    && (node->djeca[1]->svojstva->znak == "OP_LT" || node->djeca[1]->svojstva->znak == "OP_GT" 
    || node->djeca[1]->svojstva->znak == "OP_LTE" || node->djeca[1]->svojstva->znak == "OP_GTE")
    && node->djeca[2]->svojstva->znak == "<aditivni_izraz>"){
        odnosni_izraz(node->djeca[0], tablica_node);
        aditivni_izraz(node->djeca[2], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska();
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska();
    }
}

void jednakosni_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<odnosni_izraz>"){
        odnosni_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<jednakosni_izraz>" 
    && (node->djeca[1]->svojstva->znak == "OP_EQ" || node->djeca[1]->svojstva->znak == "OP_NEQ")
    && node->djeca[2]->svojstva->znak == "<odnosni_izraz>"){
        jednakosni_izraz(node->djeca[0], tablica_node);
        odnosni_izraz(node->djeca[2], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska();
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska();
    }
}

void bin_i_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<jednakosni_izraz>"){
        jednakosni_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<bin_i_izraz>" 
    && node->djeca[1]->svojstva->znak == "OP_BIN_I" && node->djeca[2]->svojstva->znak == "<jednakosni_izraz>"){
        bin_i_izraz(node->djeca[0], tablica_node);
        jednakosni_izraz(node->djeca[2], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska();
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska();
    }
}

void bin_xili_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<bin_i_izraz>"){
        bin_i_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<bin_xili_izraz>" 
    && node->djeca[1]->svojstva->znak == "OP_BIN_XILI" && node->djeca[2]->svojstva->znak == "<bin_i_izraz>"){
        bin_xili_izraz(node->djeca[0], tablica_node);
        bin_i_izraz(node->djeca[2], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska();
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska();
    }
}

void bin_ili_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<bin_xili_izraz>"){
        bin_xili_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<bin_ili_izraz>" 
    && node->djeca[1]->svojstva->znak == "OP_BIN_ILI" && node->djeca[2]->svojstva->znak == "<bin_xili_izraz>"){
        bin_ili_izraz(node->djeca[0], tablica_node);
        bin_xili_izraz(node->djeca[2], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[0]->svojstva->tip, "int") || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska();
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska();
    }
}

void log_i_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

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
            greska();
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska();
    }
}

void log_ili_izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

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
            greska();
        }
        else{
            node->svojstva->tip = "int";
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska();
    }
}

void izraz_pridruzivanja(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<log_ili_izraz>"){
        log_ili_izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<postfiks_izraz>" 
    && node->djeca[1]->svojstva->znak == "OP_PRIDRUZI" && node->djeca[2]->svojstva->znak == "<izraz_pridruzivanja>"){
        postfiks_izraz(node->djeca[0], tablica_node);
        izraz_pridruzivanja(node->djeca[2], tablica_node);
        if(node->djeca[0]->svojstva->l_izraz == false || !moze_se_pretvoriti(node->djeca[2]->svojstva->tip, node->djeca[0]->svojstva->tip)){
            greska();
        }
        else{
            node->svojstva->tip = node->djeca[0]->svojstva->tip;
            node->svojstva->l_izraz = false;
        }
    }

    else{
        greska();
    }
}

void izraz(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<izraz_pridruzivanja>"){
        izraz_pridruzivanja(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->l_izraz = node->djeca[0]->svojstva->l_izraz;
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<izraz>" 
    && node->djeca[1]->svojstva->znak == "ZAREZ" && node->djeca[2]->svojstva->znak == "<izraz_pridruzivanja>"){
        izraz(node->djeca[0], tablica_node);
        izraz_pridruzivanja(node->djeca[2], tablica_node);
        node->svojstva->tip = node->djeca[2]->svojstva->tip;
        node->svojstva->l_izraz = false;
    }

    else{
        greska();
    }
}

void slozena_naredba(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    Tablica_Node* nova_tablica = new Tablica_Node(tablica_node);

    if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "L_VIT_ZAGRADA" 
    && node->djeca[1]->svojstva->znak == "<lista_naredbi>" && node->djeca[2]->svojstva->znak == "D_VIT_ZAGRADA"){
        lista_naredbi(node->djeca[1], nova_tablica);
    }

    else if(node->djeca.size() == 5 && node->djeca[0]->svojstva->znak == "L_VIT_ZAGRADA" 
    && node->djeca[1]->svojstva->znak == "<lista_deklaracija>" && node->djeca[2]->svojstva->znak == "<lista_naredbi>"
    && node->djeca[3]->svojstva->znak == "D_VIT_ZAGRADA"){
        lista_deklaracija(node->djeca[1], nova_tablica);
        lista_naredbi(node->djeca[2], nova_tablica);
    }

    else{
        greska();
    }
}

void lista_naredbi(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<naredba>"){
        naredba(node->djeca[0], tablica_node);
    }

    else if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "<lista_naredbi>" 
    && node->djeca[1]->svojstva->znak == "<naredba>"){
        lista_naredbi(node->djeca[0], tablica_node);
        naredba(node->djeca[1], tablica_node);
    }

    else{
        greska();
    }
}

void naredba(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

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
        greska();
    }
}

void izraz_naredba(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "<izraz>" 
    && node->djeca[1]->svojstva->znak == "TOCKAZAREZ"){
        izraz(node->djeca[0], tablica_node);
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
    }

    else if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "TOCKAZAREZ"){
        node->svojstva->tip = "int";
    }

    else{
        greska();
    }
}

void naredba_grananja(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 5 && node->djeca[0]->svojstva->znak == "KR_IF" 
    && node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "<izraz>"
    && node->djeca[3]->svojstva->znak == "D_ZAGRADA" && node->djeca[4]->svojstva->znak == "<naredba>"){
        izraz(node->djeca[2], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska();
        }
        naredba(node->djeca[4], tablica_node);
    }

    else if(node->djeca.size() == 7 && node->djeca[0]->svojstva->znak == "KR_IF" 
    && node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "<izraz>"
    && node->djeca[3]->svojstva->znak == "D_ZAGRADA" && node->djeca[4]->svojstva->znak == "<naredba>"
    && node->djeca[5]->svojstva->znak == "KR_ELSE" && node->djeca[6]->svojstva->znak == "<naredba>"){
        izraz(node->djeca[2], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska();
        }
        naredba(node->djeca[4], tablica_node);
        naredba(node->djeca[6], tablica_node);
    }

    else{
        greska();
    }
}

void naredba_petlje(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 5 && node->djeca[0]->svojstva->znak == "KR_WHILE" 
    && node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "<izraz>"
    && node->djeca[3]->svojstva->znak == "D_ZAGRADA" && node->djeca[4]->svojstva->znak == "<naredba>"){
        izraz(node->djeca[2], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[2]->svojstva->tip, "int")){
            greska();
        }
        naredba(node->djeca[4], tablica_node);
    }

    else if(node->djeca.size() == 6 && node->djeca[0]->svojstva->znak == "KR_FOR" 
    && node->djeca[1]->svojstva->znak == "L_ZAGRADA" && node->djeca[2]->svojstva->znak == "<izraz_naredba>"
    && node->djeca[3]->svojstva->znak == "<izraz_naredba>" && node->djeca[4]->svojstva->znak == "D_ZAGRADA"
    && node->djeca[5]->svojstva->znak == "<naredba>"){
        izraz_naredba(node->djeca[2], tablica_node);
        izraz_naredba(node->djeca[3], tablica_node);
        if(!moze_se_pretvoriti(node->djeca[3]->svojstva->tip, "int")){
            greska();
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
            greska();
        }
        izraz(node->djeca[4], tablica_node);
        naredba(node->djeca[6], tablica_node);
    }

    else{
        greska();
    }
}

void naredba_skoka(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 2 && (node->djeca[0]->svojstva->znak == "KR_CONTINUE" 
    || node->djeca[0]->svojstva->znak == "KR_BREAK")
    && node->djeca[1]->svojstva->znak == "TOCKAZAREZ"){
        if(!jel_u_petlji(node)){
            greska();
        }
        node->svojstva->tip = "int";
    }

    else if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "KR_RETURN"
    && node->djeca[1]->svojstva->znak == "TOCKAZAREZ"){
        string tip = jel_u_funkciji(node);
        if(tip != "void"){
            greska();
        }
        node->svojstva->tip = "int";
    }

    else if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "KR_RETURN"
    && node->djeca[1]->svojstva->znak == "<izraz>" && node->djeca[2]->svojstva->znak == "TOCKAZAREZ"){
        string tip = jel_u_funkciji(node);
        if(tip == "void"){
            greska();
        }
        izraz(node->djeca[1], tablica_node);
        node->svojstva->tip = tip;
    }

    else{
        greska();
    }
}

void prijevodna_jedinica(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    Tablica_Node* nova_tablica = new Tablica_Node(tablica_node);

    if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "<prijevodna_jedinica>" 
    && node->djeca[1]->svojstva->znak == "<vanjska_deklaracija>"){
        prijevodna_jedinica(node->djeca[0], nova_tablica);
        vanjska_deklaracija(node->djeca[1], nova_tablica);
    }

    else if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<vanjska_deklaracija>"){
        vanjska_deklaracija(node->djeca[0], nova_tablica);
    }

    else{
        greska();
    }
}

void vanjska_deklaracija(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<definicija_funkcije>"){
        definicija_funkcije(node->djeca[0], tablica_node);
    }

    else if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<deklaracija>"){
        deklaracija(node->djeca[0], tablica_node);
    }

    else{
        greska();
    }
}

void definicija_funkcije(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 6 && node->djeca[0]->svojstva->znak == "<ime_tipa>" 
    && node->djeca[1]->svojstva->znak == "IDN" && node->djeca[2]->svojstva->znak == "L_ZAGRADA"
    && node->djeca[3]->svojstva->znak == "KR_VOID" && node->djeca[4]->svojstva->znak == "D_ZAGRADA"
    && node->djeca[5]->svojstva->znak == "<slozena_naredba>"){
        ime_tipa(node->djeca[0], tablica_node);
        if(node->djeca[0]->svojstva->konst){
            greska();
        }

        Node* prijasnja_definicija = provjeri_tablicu(node->djeca[1]->svojstva->leks_jedinka, tablica_node);
        if(prijasnja_definicija != nullptr){
            if(prijasnja_definicija->svojstva->argumenti.size() != 0){
                greska();
            }
            if(prijasnja_definicija->svojstva->tip.find(" -> ") != string::npos 
            || split(prijasnja_definicija->svojstva->tip, " -> ")[1] != node->djeca[0]->svojstva->tip){
                greska();
            }
        }

        node->svojstva->tip = "funkcija() -> " + node->djeca[0]->svojstva->tip;
        node->svojstva->argumenti = {};
        node->svojstva->leks_jedinka = node->djeca[1]->svojstva->leks_jedinka;
        tablica_node->zapis.insert({node->djeca[1]->svojstva->leks_jedinka, node});
        slozena_naredba(node->djeca[5], tablica_node);
    }

    else if(node->djeca.size() == 6 && node->djeca[0]->svojstva->znak == "<ime_tipa>"
    && node->djeca[1]->svojstva->znak == "IDN" && node->djeca[2]->svojstva->znak == "L_ZAGRADA"
    && node->djeca[3]->svojstva->znak == "<lista_parametara>" && node->djeca[4]->svojstva->znak == "D_ZAGRADA"
    && node->djeca[5]->svojstva->znak == "<slozena_naredba>"){
        ime_tipa(node->djeca[0], tablica_node);
        if(node->djeca[0]->svojstva->konst){
            greska();
        }

        lista_parametara(node->djeca[3], tablica_node);
        Node* prijasnja_definicija = provjeri_tablicu(node->djeca[1]->svojstva->leks_jedinka, tablica_node);
        if(prijasnja_definicija != nullptr){
            if(prijasnja_definicija->svojstva->argumenti.size() != node->djeca[3]->svojstva->argumenti.size()){
                greska();
            }
            for(int i = 0; i < node->djeca[3]->svojstva->argumenti.size(); i++){
                if(prijasnja_definicija->svojstva->argumenti[i] != node->djeca[3]->svojstva->argumenti[i]){
                    greska();
                }
            }
            if(prijasnja_definicija->svojstva->tip.find(" -> ") != string::npos 
            || split(prijasnja_definicija->svojstva->tip, " -> ")[1] != node->djeca[0]->svojstva->tip){
                greska();
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
        slozena_naredba(node->djeca[5], tablica_node);
    }
    else{
        greska();
    }
}

void lista_parametara(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

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
                greska();
            }
        }
        node->svojstva->argumenti = node->djeca[0]->svojstva->argumenti;
        node->svojstva->argumenti.push_back(node->djeca[2]->svojstva->tip);
        node->svojstva->argumenti_imena = node->djeca[0]->svojstva->argumenti_imena;
        node->svojstva->argumenti_imena.push_back(node->djeca[2]->svojstva->leks_jedinka);
    }

    else{
        greska();
    }
}

void deklaracija_parametra(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "<ime_tipa>" 
    && node->djeca[1]->svojstva->znak == "IDN"){
        ime_tipa(node->djeca[0], tablica_node);
        if(node->djeca[0]->svojstva->tip == "void"){
            greska();
        }
        node->svojstva->tip = node->djeca[0]->svojstva->tip;
        node->svojstva->leks_jedinka = node->djeca[1]->svojstva->leks_jedinka;
    }

    else if(node->djeca.size() == 4 && node->djeca[0]->svojstva->znak == "<ime_tipa>"
    && node->djeca[1]->svojstva->znak == "IDN" && node->djeca[2]->svojstva->znak == "L_UGL_ZAGRADA"
    && node->djeca[3]->svojstva->znak == "D_UGL_ZAGRADA"){
        ime_tipa(node->djeca[0], tablica_node);
        if(node->djeca[0]->svojstva->tip == "void"){
            greska();
        }
        node->svojstva->tip = "niz(" + node->djeca[0]->svojstva->tip + ")";
        node->svojstva->leks_jedinka = node->djeca[1]->svojstva->leks_jedinka;
    }

    else{
        greska();
    }
}

void lista_deklaracija(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 1 && node->djeca[0]->svojstva->znak == "<deklaracija>"){
        deklaracija(node->djeca[0], tablica_node);
    }

    else if(node->djeca.size() == 2 && node->djeca[0]->svojstva->znak == "<lista_deklaracija>" 
    && node->djeca[1]->svojstva->znak == "<deklaracija>"){
        lista_deklaracija(node->djeca[0], tablica_node);
        deklaracija(node->djeca[1], tablica_node);
    }

    else{
        greska();
    }
}

void deklaracija(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

    if(node->djeca.size() == 3 && node->djeca[0]->svojstva->znak == "<ime_tipa>" 
    && node->djeca[1]->svojstva->znak == "<lista_init_deklaratora>" && node->djeca[2]->svojstva->znak == "TOCKAZAREZ"){
        ime_tipa(node->djeca[0], tablica_node);
        node->djeca[1]->svojstva->ntip = node->djeca[0]->svojstva->tip;
        lista_init_deklaratora(node->djeca[1], tablica_node);
    }

    else{
        greska();
    }
}

void lista_init_deklaratora(Node* node, Tablica_Node* tablica_node){
    if(node->svojstva == nullptr) greska();

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
        greska();
    }
}
//------------------------------------------------------------------------------------------------
//------------------------------------------------------------------------------------------------

int main(void){
    

    return 0;
}
