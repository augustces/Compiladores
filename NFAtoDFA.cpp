#include <map>
// as outras bibliotecas estão dentro desse arquivo a baixo
#include "toNFA.cpp"

// função que preenche o alfabeto
std::vector<char> preencherAlfabeto(){
     //criar o alfabeto
    std::vector<char> alfabeto; //letras e números
    //adicionar letras e números
    for(int i{48}; i< 125; i++){
        char c =i;
        if ((i >= 48 && i <= 57) || (i>=65 && i<=90) || (i>=97 && i<=122)){
            alfabeto.push_back(c);
        }
    }
    //adicionar os outros caracteres especiais
    alfabeto.push_back(';');
    alfabeto.push_back('=');
    alfabeto.push_back('!');
    alfabeto.push_back('+');
    alfabeto.push_back('-');
    alfabeto.push_back(',');
    alfabeto.push_back('*');
    alfabeto.push_back('%');
    alfabeto.push_back('{');
    alfabeto.push_back('}');
    alfabeto.push_back('(');
    alfabeto.push_back(')');
    alfabeto.push_back('/');
    alfabeto.push_back('&');
    alfabeto.push_back('|');
    alfabeto.push_back('<');
    alfabeto.push_back('>');
    alfabeto.push_back('"');

    return alfabeto;

}

struct DFA{
    //estados presentes na estrutura
    std::vector<Estado*> estados;
    //posição do estado inicial no vetor
    int posInicial{0};
};

// estrutura auxiliar que irá servir como campos da tabela
struct CampoTabelaDFA{
    std::vector<Estado*> conjuntoOrigem; // representa a linha da tabela
    char charTransicao; // representa a coluna da tabela
    std::vector<Estado*> conjuntoDestino; // representa o campo da tabela, relacionado a linha  e coluna
};

// controlador que organiza as funções e atributos necessários para a conversão
struct ControleDFA{
    std::vector<CampoTabelaDFA*> tabelaDeConversao; // tabela
    NFA* nfa1; // Autômato finito não determinístico que será convertido
    std::vector<char> alfabeto; // alfabeto
    DFA* DFAFinal; // autômato finito determinístico convertido
    std::vector<std::pair<std::vector<Estado*>,Estado*>> relacao; // relações entre um conjunto de estados e um estado, estrutura auxiliar

// função que junta dois vetores de maneira que nenhum elemento esteja repetido
    std::vector<Estado*> juntarConjuntos(std::vector<Estado*> conjunto1,std::vector<Estado*> conjunto2){
        for(Estado* estado : conjunto2){
            if(this->procuraPosicao(conjunto1, estado) == -1)
                conjunto1.push_back(estado);
        }
        return conjunto1;
    }

    ControleDFA(NFA* nfa){
        this->DFAFinal = new DFA();
        this->nfa1 = nfa;
        // coloco na tabela um conjunto de todas as epslon-transições de um estado 
        for(Estado* estado : nfa->estados){
            CampoTabelaDFA* campo = new CampoTabelaDFA();
            campo->charTransicao = 0;
            campo->conjuntoOrigem = this->EstadosSimuntaneos(estado, 0);
            campo->conjuntoDestino= campo->conjuntoOrigem;
            bool achou = false;
            // verifico se o conjunto não está repetido na tabela
            for( CampoTabelaDFA* campoTemp : this->tabelaDeConversao){
                if ((campoTemp->charTransicao == campo->charTransicao && this->igualdadeVectors(campoTemp->conjuntoOrigem, campo->conjuntoOrigem) && this->igualdadeVectors(campoTemp->conjuntoDestino, campo->conjuntoDestino))){
                    achou = true;
                    break;
                }
            }
            // crio um estado referente a cada conjunto
            if(!achou){
                this->tabelaDeConversao.push_back(campo);
                Estado* q = new Estado();
                this->relacao.push_back({campo->conjuntoOrigem, q}); 
            }            
        }

        this->alfabeto = preencherAlfabeto();

    // descubro os destinos de cada conjunto de estados (que já estão na tabela) utilizando cada caractere do alfabeto
        for(int it{0}; it< this->relacao.size(); it+=1){
            std::pair<std::vector<Estado*>, Estado*> par = this->relacao[it]; 
            for( char c : this->alfabeto){
                std::vector<Estado*> destinos; 
                for(Estado* estado : par.first){ // pego os estados alcançavéis com tal char 
                    std::vector<Estado*> destinosClosure =this->EstadosAlcancaveis(estado, c);
                    if(destinosClosure.empty()){
                        continue;
                    }
                   std::vector<Estado*> destinosTemp; 
                    for(Estado* estadoTemp : destinosClosure){ // junto em um conjutno só todas as epslon-transições de cada estado alcançável
                        destinosTemp = juntarConjuntos(destinosTemp, this->EstadosSimuntaneos(estadoTemp,0));
                    }
                    destinos = this->juntarConjuntos(destinos, destinosTemp);
                    
                }
                if(destinos.empty()){
                    continue;
                } // a cada nova transição, adiciono na tabela qual foi o conjunto de origem, o char utilizado e o conjunto de destino
                CampoTabelaDFA* campoNovo = new CampoTabelaDFA();
                campoNovo->charTransicao = c;
                campoNovo->conjuntoOrigem = par.first;
                campoNovo->conjuntoDestino = destinos;
                this->tabelaDeConversao.push_back(campoNovo);
                bool achou = false;
                for(std::pair<std::vector<Estado*>, Estado*> par : this->relacao){
                    if (this->igualdadeVectors(par.first, campoNovo->conjuntoDestino)) {
                        achou = true;
                        break;
                    }
                } // se o conjunto de destino não estiver incluso no vetor de estados que formará o DFA, será criado um estado que o representará
                if (!achou){
                    Estado* q = new Estado();
                    this->relacao.push_back({campoNovo->conjuntoDestino, q});
                }
            }
        }
        converterParaDFA();
    }

    //função auxiliar que encontra o estado que representa um determinado conjunto
    Estado* acharEstado(std::vector<Estado*> conjuntoProcurado){
        for(std::pair<std::vector<Estado*>,Estado*> par : this->relacao){
            if(this->igualdadeVectors(conjuntoProcurado, par.first)){
                return par.second;
            }
        }
        return nullptr;
    }

// função que verifica a igualdade entre dois conjuntos
    bool igualdadeVectors(std::vector<Estado*> vet1, std::vector<Estado*> vet2){
        if (vet1.size() > vet2.size() || vet1.size() < vet2.size()){
            return false;
        }
        for(Estado* estado : vet1){
            if(this->procuraPosicao(vet2, estado) == -1)
                return false;
        }
        return true;
    }

// função que converte a tabela para um DFA
    void converterParaDFA(){
        
        for(std::pair<std::vector<Estado*>, Estado*> par : this->relacao){
            for(Estado* estado : par.first){ // descobre quais estados são finais de acordo com o conjunto de estados que esse representa
                if(estado->final){
                    par.second->final = true;
                }
            }
            for(char c : this->alfabeto){ // para cada estado, faço as transições entre os outros estados
                for( CampoTabelaDFA* campo : this->tabelaDeConversao){
                    if (this->igualdadeVectors(campo->conjuntoOrigem, par.first) && campo->charTransicao == c){
                        par.second->transicoes.push_back({c,this->acharEstado(campo->conjuntoDestino)}); 
                    }
                }
            }
        }
// adiciono os estados no DFA, já com suas transições e informando quem é final
        for(std::pair<std::vector<Estado*>, Estado*> par : this->relacao){
            if(this->procuraPosicao(this->DFAFinal->estados, par.second) == -1)
                this->DFAFinal->estados.push_back(par.second);
        }
    }

// função auxiliar que encontra a posição de um estado dentro de um vetor
    int procuraPosicao(std::vector<Estado*> vet, Estado * estado){
        int t{0};
        for(Estado* estadoTemp : vet){
            if(estadoTemp == estado){
                return t;
            }
            t+=1;
        }
        return -1;
    }

// função que printa o DFA, sendo representado por um conjunto de estados, o estado inicial, os estados finais e suas transições
     void printarDFA(){
        std::string str{""}; //string final com todos os dados inseridos
        str+= "Estados: ";
        // coloca na string todos os estados presentes no vetor de estados,
        // os identifica com um número iniciando de 0, que é o estado inicial
        for(int i{0}; i < this->DFAFinal->estados.size();i++){
            str+= "q" + std::to_string(i) + ", ";
        }
        str+='\n';

        str += "Estado inicial: q" + std::to_string(this->DFAFinal->posInicial) + "\n";
        std::vector<Estado*> finais;
        str += "Estados finais: ";
        // procura no vetor quais os estados finais para printar
        int t{0};
        for(Estado* estado : this->DFAFinal->estados){
            if(estado->final){
                str += "q" + std::to_string(t) + ", ";
                finais.push_back(estado);
            }
            t+=1;
        }
        str += '\n';
        str += "Transicoes: ";
        // escreve odas as transformações na string na estrutura:
        // delta( estado de origem, char de transição, estado de destino)
        t =0;
        for(Estado* estado : this->DFAFinal->estados){
            for(int j{0}; j < estado->transicoes.size();j++){
                std::pair<char, Estado*> transicao = estado->transicoes[j];
                str += "delta(q" + std::to_string(t) + ",";
                str += transicao.first;
                str += ",q";

                str += std::to_string(this->procuraPosicao(this->DFAFinal->estados, transicao.second));
                str += "); ";
            }            
            t+=1;
        }
        std::cout << str << '\n';
    }

// função que determina os estados que compõem uma epson transição
    std::vector<Estado*> EstadosSimuntaneos(Estado * estado, char c){
        std::vector<Estado*> conjunto;
        conjunto.push_back(estado);
        std::vector<Estado*> conjuntoCopia;
        while(conjunto != conjuntoCopia){
            conjuntoCopia = conjunto;
            for(Estado* estadoTemp : conjunto)
                for(std::pair<char, Estado*> par : estadoTemp->transicoes){
                    if (par.first == c){
                        if (this->procuraPosicao(conjunto, par.second) == -1)
                            conjunto.push_back(par.second);
                    }
                }
        }
        return conjunto;
    }

// função auxiliar que determina quais estados são alcançavéis a partir de um estado de partida e um caractere
    std::vector<Estado*> EstadosAlcancaveis(Estado * estado, char c){
        std::vector<Estado*> conjunto;
        for(std::pair<char, Estado*> par : estado->transicoes){
            if (par.first == c){
                if (this->procuraPosicao(conjunto, par.second) == -1)
                    conjunto.push_back(par.second);
            }
        }
        return conjunto;
    }
};