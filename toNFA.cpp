#include <iostream>
#include <vector>
#include <string>
#include <stack>
#include <string>

//um estado e suas transições
struct Estado{
    // todas as transições do estado atual para os demais com um determinado char
    // para representar o epslon, o char recebe 0 
    std::vector<std::pair<char,Estado*>> transicoes;
    bool final{false}; //se um estado for final ou não
};

// representa a estrutura do NFA (ou AFN)
struct NFA{
    //estados presentes na estrutura
    std::vector<Estado*> estados;
    //posição do estado inicial no vetor
    int posInicial{0};
};

// controlador de operações de conversão de ER pra NFA
struct  ControleNFA {
    // representa o NFA final, após todos as ERs serem inseridas e convertidas
    NFA * AFN_Final;
    // construtor do controlador que faz um estado inicial q0 que vai para
    // os estados iniciais dos demais AFNs com epslon 
    ControleNFA(){
        Estado* q0 = new Estado();
        this->AFN_Final->estados.push_back(q0);
        this->AFN_Final->posInicial = 0;
    }

    // função que adiciona uma ER, converte para NFA, e liga o estado inicial
    // do autômato final para o inicial do NFA com epslon
    void addER(std::string ER){
        NFA* nfa = this->concatenarNFA(ER);
        this->AFN_Final->estados[this->AFN_Final->posInicial]->transicoes.push_back({0, nfa->estados[nfa->posInicial]});
        for(Estado * estado : nfa->estados){
            this->AFN_Final->estados.push_back(estado);
        }
    }

    // função auxiliar que descobre a posição de determinado estado no vetor
    // de estados, caso não o ache, retorna -1
    int procuraPosicao(NFA* nfa, Estado * estado){
        for(int i{0}; i < nfa->estados.size(); i++){
            if(nfa->estados[i] == estado){
                return i;
            }
        }
        return -1;
    }

// função auxiliar que printa um NFA, representado por:
/* - um conjunto finito de estados (todos os estados presentes no NFA
    - o estados inicial
    - todos os estados finais
    - todas as transições, seja com epslon seja com outro caractere )*/
    void printarNFA(){
        std::string str{""}; //string final com todos os dados inseridos
        str+= "Estados: ";
        // coloca na string todos os estados presentes no vetor de estados,
        // os identifica com um número iniciando de 0, que é o estado inicial
        for(int i{0}; i < this->AFN_Final->estados.size();i++){
            Estado* estado = this->AFN_Final->estados[i];
            str+= "q" + std::to_string(i) + ", ";
        }
        str+='\n';
        // guarda o estado final e informa qual seu identificador(0)
        Estado* state = this->AFN_Final->estados[this->AFN_Final->posInicial];
        str += "Estado inicial: q" + std::to_string(this->AFN_Final->posInicial) + "\n";
        std::vector<Estado*> finais;
        str += "Estados finais: ";
        // procura no vetor quais os estados finais para printar
        for(int i{0}; i < this->AFN_Final->estados.size();i++){
            Estado* estado = this->AFN_Final->estados[i];
            if(estado->final){
                str += "q" + std::to_string(i) + ", ";
                finais.push_back(estado);
            }
        }
        str += '\n';
        str += "Transicoes: ";
        // escreve odas as transformações na string na estrutura:
        // delta( estado de origem, char de transição, estado de destino)
        for(int i{0}; i < this->AFN_Final->estados.size();i++){
            Estado* estado = this->AFN_Final->estados[i];
            for(int j{0}; j < estado->transicoes.size();j++){
                std::pair<char, Estado*> transicao = estado->transicoes[j];
                str += "delta(q" + std::to_string(i) + ",";
                if(transicao.first > 0){
                    str += transicao.first;
                    str += ",q";
                }
                else{
                    str+="epslon,q";
                }
                str += std::to_string(procuraPosicao(this->AFN_Final, transicao.second));
                str += "); ";
            }            
        }
        std::cout << str << '\n';
    }

    /* junta todos os NFAs construídos até o momento e realiza as operações da
    notação de expressões regulares (ou, concatenação ou *)
    */
    NFA* operacoesER(std::stack<NFA*> pilhaNFA, std::stack<char> pilhaChar){
        // auxiliares
        std::stack<NFA*> bufferNFA; // buffer auxiliar de NFAs
        std::stack<char> operandos; // pilha de caracteres de ER (|, [,],*,.) e NFA(representados por 0)

        // lê toda a estrutura dos NFAs, símbolo por símbolo
        while(!(pilhaChar.empty())){
            char simbolo = pilhaChar.top();
            pilhaChar.pop();
            //verifica qual símbolo é
            if (simbolo == '['){
                if (!operandos.empty() && operandos.top() == 0){
                    operandos.push('.'); // operando de concatenação
                }
                operandos.push(simbolo);
                continue;
            }
            else if(simbolo == '|'){
                operandos.push(simbolo);
            }
            else if(simbolo == '*'){
                // cria um NFA resultante, em que há um estado inicial e um final
                // é necessário ligar o inicial do NFA resultante no inicial do NFA 
                // que está operando com o *; e liga o final do NFA com o final do
                // NFA resultante, tudo isso com epslon transição. Empilha o NFA resultante

                NFA* result = new NFA();
                Estado* q0 = new Estado();
                Estado* q1 = new Estado();
                q1->final = true;
                result->estados.push_back(q0);
                result->estados.push_back(q1);
                result->posInicial = 0;
                NFA* nfa0 = pilhaNFA.top();
                pilhaNFA.pop();
                q0->transicoes.push_back({0, nfa0->estados[nfa0->posInicial] });
                for(Estado* estado : nfa0->estados){
                    if (estado->final){
                        estado->final = false;
                        estado->transicoes.push_back({0,q1});
                    }
                    result->estados.push_back(estado);
                }
                q0->transicoes.push_back({0,q1});
                q1->transicoes.push_back({0,q0});
                pilhaNFA.push(result);
                pilhaChar.push(0);
                bufferNFA.push(result);
            }
            else if(simbolo == 0){
                // armazena o NFA tanto no buffer como no operandos, sendo representado por 0
                bufferNFA.push(pilhaNFA.top());
                operandos.push(0);
                pilhaNFA.pop();
            }
            else if (simbolo == ']'){
                char temp = operandos.top();
                operandos.pop();
                // realiza operações até fechar o cochete
                while (!operandos.empty() && temp != '[') {
                    if( operandos.top() == '|'){
                        /* cria um NFA resultante, com um estado inicial e um final. Pego os
                        dois NFAs de cada lado do |, ligo o inicial do NFA resultante com os
                        iniciais de cada um dos outros dois, e o final de cada um dos outros
                        dois NFAs com o final do NFA resultante. Empilha o NFA resultante
                        */
                        operandos.pop();
                        NFA* result = new NFA();
                        Estado* q0 = new Estado();
                        Estado* q1 = new Estado();
                        q1->final = true;
                        result->estados.push_back(q0);
                        result->estados.push_back(q1);
                        result->posInicial = 0;
                        NFA* nfa1 = bufferNFA.top();
                        bufferNFA.pop();
                        NFA* nfa2 = bufferNFA.top();
                        bufferNFA.pop();
                        operandos.pop();
                        result->estados[result->posInicial]->transicoes.push_back({0, nfa1->estados[nfa1->posInicial]});
                        result->estados[result->posInicial]->transicoes.push_back({0, nfa2->estados[nfa2->posInicial]});
                        for(Estado* estado : nfa1->estados){
                            if (estado->final){
                                estado->final = false;
                                estado->transicoes.push_back({0,q1});
                            }
                            result->estados.push_back(estado);
                        }
                        for(Estado* estado : nfa2->estados){
                            if (estado->final){
                                estado->final = false;
                                estado->transicoes.push_back({0,q1});
                            }
                            result->estados.push_back(estado);
                        }
                        bufferNFA.push(result);
                        temp = operandos.top();
                        operandos.pop();
                        operandos.push(0);
                    }
                    else if( operandos.top() == '.'){
                        // se for uma concatenação, pego o primeiro NFA e ligo seu final ao inicial
                        // do segundo NFA, o final do 1° deixa de ser final
                        operandos.pop();
                        NFA* nfa2 = bufferNFA.top();
                        bufferNFA.pop();
                        NFA* nfa1 = bufferNFA.top();
                        bufferNFA.pop();
                        operandos.pop();
                        for(Estado* estado : nfa1->estados){
                            if(estado->final){
                                estado->final = false;
                                estado->transicoes.push_back({0, nfa2->estados[nfa2->posInicial]});
                            }
                        }
                        for(Estado* estado : nfa2->estados){
                            nfa1->estados.push_back(estado);
                        }
                        temp = operandos.top();
                        bufferNFA.push(nfa1);
                    }
                    else if( operandos.top() == '['){
                        operandos.pop();
                    }
                }
            }
        }
        return bufferNFA.top();
    }

    //criar NFA para cada letra e deixar os comandos separados (|, *, .)
    std::pair<std::stack<char>, std::stack<NFA*>> criarNFA(std::string ER){
        std::stack<NFA*> pilhaNFA; // pilha de NFAs
        std::stack<char> pilhaChar; // pilha com os demais caracteres da notação de ER

        for(int i{0}; i < ER.length();i++){
            char c = ER[i];
            if (c == '['){
                pilhaChar.push(c);
            }
            else if(c == '^'){
                if (ER[i + 1] == '|'){
                    pilhaChar.push(ER[i+1]);
                }
                else if(ER[i + 1] == '*'){
                    pilhaChar.push(ER[i+1]);
                }
            }
            else if(c == ']'){
                pilhaChar.push(c);
            }
            else if( c == '|' && i >0 && ER[i - 1] != '^' && (i + 1) < ER.length() && ER[i + 1]=='|'){
                // caso do ||
                NFA* nfaTemp = new NFA();
                Estado* q0= new Estado();
                Estado * q1 = new Estado();
                Estado * q2 = new Estado();
                nfaTemp->estados.push_back(q0);
                nfaTemp->posInicial=0;
                nfaTemp->estados.push_back(q1);
                nfaTemp->estados.push_back(q2);
                q2->final=true;
                q0->transicoes.push_back({'|', q1});
                q1->transicoes.push_back({'|', q2});
                pilhaNFA.push(nfaTemp);
                pilhaChar.push(0);
            }
            else if (c == '&' && (i + 1) < ER.length() && ER[i + 1]=='&'){
                //caso do &&
                NFA* nfaTemp = new NFA();
                Estado* q0= new Estado();
                Estado * q1 = new Estado();
                Estado * q2 = new Estado();
                nfaTemp->estados.push_back(q0);
                nfaTemp->posInicial=0;
                nfaTemp->estados.push_back(q1);
                nfaTemp->estados.push_back(q2);
                q2->final=true;
                q0->transicoes.push_back({'&', q1});
                q1->transicoes.push_back({'&', q2});
                pilhaNFA.push(nfaTemp);
                pilhaChar.push(0);
            }
            else if(c == '!' && (i + 1) < ER.length() && ER[i + 1]=='='){
                // caso do !=
                NFA* nfaTemp = new NFA();
                Estado* q0= new Estado();
                Estado * q1 = new Estado();
                Estado * q2 = new Estado();
                nfaTemp->estados.push_back(q0);
                nfaTemp->posInicial=0;
                nfaTemp->estados.push_back(q1);
                nfaTemp->estados.push_back(q2);
                q2->final=true;
                q0->transicoes.push_back({'!', q1});
                q1->transicoes.push_back({'=', q2});
                pilhaNFA.push(nfaTemp);
                pilhaChar.push(0);
                
            }
            else if (c == '=' && i>0 && ER[i-1] != '!'){
                // caso do =
                NFA* nfaTemp = new NFA();
                Estado* q0= new Estado();
                Estado * q1 = new Estado();
                nfaTemp->posInicial=0;
                q1->final=true;
                q0->transicoes.push_back({c, q1});
                nfaTemp->estados.push_back(q0);
                nfaTemp->estados.push_back(q1);
                pilhaNFA.push(nfaTemp);
                pilhaChar.push(0);
                
            }
            else if (c == '*' && ER[i - 1] == '^'){continue;}
            else if (c == '|'){ continue;}
            else{
                NFA* nfaTemp = new NFA();
                Estado* q0= new Estado();
                Estado * q1 = new Estado();
                nfaTemp->posInicial=0;
                q1->final=true;
                q0->transicoes.push_back({c, q1});
                nfaTemp->estados.push_back(q0);
                nfaTemp->estados.push_back(q1);
                pilhaNFA.push(nfaTemp);
                pilhaChar.push(0);
            }
        }
        return {pilhaChar, pilhaNFA};
    }

    /* após cada caractere ter seu NFA, concateno os NFAs que estão vizinhos,
    gerando um NFA e mantendo-o na estrutura da fórmula, depois envio para
    a outra função para realizar as demais operações
    */
    NFA* concatenarNFA(std::string ER){
        std::pair<std::stack<char>, std::stack<NFA*>> pilhas = this->criarNFA(ER);
        std::stack<char> pilhaChar;
        std::stack<NFA*> pilhaNFA ;
        std::stack<NFA*> bufferNFA;

        while(!(pilhas.first.empty())){
            char value = pilhas.first.top();
            pilhas.first.pop();
            if(value > 0){
                if(!(bufferNFA.empty())){
                    NFA* nfaResul = bufferNFA.top();
                    bufferNFA.pop();
                    while(!bufferNFA.empty()){
                        NFA* nfa1 = bufferNFA.top();
                        bufferNFA.pop();
                        for(Estado* estado : nfaResul->estados){
                            if(estado->final){
                                estado->final = false;
                                estado->transicoes.push_back({0, nfa1->estados[nfa1->posInicial]});
                            }
                        }
                        for(Estado* estado : nfa1->estados){
                            nfaResul->estados.push_back(estado);
                        }
                    }
                    pilhaChar.push(0);
                    pilhaNFA.push(nfaResul);
                }
                pilhaChar.push(value);
            }
            else{
                bufferNFA.push(pilhas.second.top());
                pilhas.second.pop();
            }
        }
        return this->operacoesER(pilhaNFA, pilhaChar);
    }

};