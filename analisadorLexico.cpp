#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <string>
#include <fstream>
#include <map>
#include <sstream>

struct Estado{
    // todas as transições do estado atual para os demais com um determinado char
    // para representar o epslon, o char recebe 0 
    std::vector<std::pair<char,Estado*>> transicoes;
    bool final{false}; //se um estado for final ou não
};

struct DFA{
    //estados presentes na estrutura
    std::vector<Estado*> estados;
    //posição do estado inicial no vetor
    int posInicial{0};
};

void percorrer(DFA* dfa, std::string str){
    std::ofstream fout;
    fout.open("analiseLexica.txt");
    std::vector<std::string> tokens;
    tokens ={
        "INT", "STRING", "ID", "CONST", "PV", "EQ", "ADD", "MINUS",
        "TIMES", "DIV", "MOD", "NUM", "IF", "ELSE", "RETURN", "PE", "PD",
        "VIR", "CE", "CD", "MENOR", "MAIOR", "E", "OU", "DIF"
    };
    Estado* estadoAtual = dfa->estados.front();
    int it{0};
    for(int i{0}; i < estadoAtual->transicoes.size(); i++){
        if(estadoAtual->transicoes[i].first == str.front()){
            it = i;
            break;
        }
    }
    for(char c : str){
       if(c == ' ' || c == '\n') {
        if(!estadoAtual->final) {
            fout.seekp(std::ios::beg);
            fout << "ERRO\n";
            break;
        }   
        else{
            fout << tokens[it];
            std::cout << tokens[it] << c;
        }
       }
    }
    fout.close();
}

DFA* lerDFA(){
    DFA* dfa = new DFA();
    std::fstream fin;
        fin.open("dfa.txt");
        std::string linha;
        for(int i{0}; i < 2; i++){
            getline(fin, linha);
            int index{0};
            std::stringstream ss (linha);
            if(i == 0){
                while(!ss.eof()){
                    ss >> index;
                    if(ss.eof())
                        break;
                    Estado* q = new Estado();
                    dfa->estados.push_back(q);
                }
            }
            else {
                while(!ss.eof()){
                    ss >> index;
                    if (ss.eof())
                        break;
                    dfa->estados[index]->final = true;
                }
            }
        }

        while(getline(fin, linha)){
            std::stringstream ss (linha);
            int origem, destino;
            char cTransicao;
            ss >>origem >> cTransicao >> destino;
            dfa->estados[origem]->transicoes.push_back({cTransicao, dfa->estados[destino]});
        }
        fin.close();
        return dfa;
}

void lerEntrada(){
    std::fstream fin;
        fin.open("dfa.txt");
        std::string linha;
        while(getline(fin, linha)){
            percorrer(lerDFA(), linha);
        }
        fin.close();
}

int main(){
    lerEntrada();
    return 0;
}