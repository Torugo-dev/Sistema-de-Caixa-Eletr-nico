#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARQUIVO "contas.dat"

typedef enum{
    CONTA_ENCERRADA,
    CONTA_ATIVA
} SituacaoConta;

typedef struct{
    long numero;
    char titular[100];
    char cpf[15];
    double saldo;
    SituacaoConta situacao;
} Conta;

#define ARQUIVO_TRANS "transacoes.dat"

typedef enum{
    DEPOSITO,
    SAQUE,
    TRANSFERENCIA_ENVIADA,
    TRANSFERENCIA_RECEBIDA
} TipoTransacao;

typedef struct{
    long conta;
    long contaRelacionada;
    TipoTransacao tipo;
    double valor;
} Transacao;

void registrarTransacao(long conta, long relacionada, TipoTransacao tipo, double valor){
    FILE *arquivo;
    Transacao t;

    arquivo = fopen(ARQUIVO_TRANS, "ab");
    if (arquivo == NULL){
        printf("Erro ao abrir o historico.\n");
        return;
    }

    t.conta = conta;
    t.contaRelacionada = relacionada;
    t.tipo = tipo;
    t.valor = valor;

    if (fwrite(&t, sizeof(Transacao), 1, arquivo) != 1)
        printf("Erro ao gravar a transacao.\n");

    fclose(arquivo);
}

int buscarConta(FILE *arquivo, long numeroConta, Conta *conta){
    Conta temp;

    rewind(arquivo);
    while (fread(&temp, sizeof(Conta), 1, arquivo) == 1){
        if (temp.numero == numeroConta){
            *conta = temp;
            return 1;
        }
    }
    return 0;
}

int atualizarConta(FILE *arquivo, const Conta *conta){
    Conta temp;
    long posicao;

    rewind(arquivo);
    while (1){
        posicao = ftell(arquivo);
        if (fread(&temp, sizeof(Conta), 1, arquivo) !=1)
            break;

        if (temp.numero == conta -> numero){
            fseek(arquivo, posicao, SEEK_SET);
            if (fwrite(conta, sizeof(Conta), 1, arquivo) !=1)
                return 0;
            fflush(arquivo);
            return 1;
        }
    }
    return 0;
}

void cadastrarConta(void){
    FILE *arquivo;
    Conta nova, existente;

    arquivo = fopen(ARQUIVO, "rb+");
    if (arquivo == NULL){
        arquivo = fopen(ARQUIVO, "wb+");
        if (arquivo == NULL){
            printf("Erro ao abrir o arquivo.\n");
            return;
        }
    }

    printf("Número da conta: ");
    scanf("%ld", &nova.numero);
    getchar();

    if (buscarConta(arquivo, nova.numero, &existente)){
        printf("Já existe uma conta com esse número.\n");
        fclose(arquivo);
        return;
    }

    printf("Nome do titular: ");
    fgets(nova.titular, sizeof(nova.titular), stdin);
    nova.titular[strcspn(nova.titular, "\n")] = '\0';

    printf("CPF: ");
    fgets(nova.cpf, sizeof(nova.cpf), stdin);
    nova.cpf[strcspn(nova.cpf, "\n")] = '\0';

    nova.saldo = 0.0;
    nova.situacao = CONTA_ATIVA;

    fseek(arquivo, 0, SEEK_END);
    if (fwrite(&nova, sizeof(Conta), 1, arquivo) !=1)
        printf("Erro ao cadastrar a conta.\n");
    else
        printf("Conta cadastrada com sucesso!\n");
    fclose(arquivo);
}

void consultarConta(void){
    FILE *arquivo;
    Conta conta;
    long numero;

    arquivo = fopen(ARQUIVO, "rb");
    if (arquivo == NULL){
        printf("Nenhuma conta cadastrada ainda.\n");
        return;
    }

    printf("Número da conta: ");
    scanf("%ld", &numero);
    getchar();
    
    if (buscarConta(arquivo, numero, &conta)){
        printf("\n--------Dados da conta--------\n");
        printf("Número: %ld\n", conta.numero);
        printf("Titular: %s\n", conta.titular);
        printf("CPF: %s\n", conta.cpf);
        printf("Saldo: R$ %.2f\n", conta.saldo);
        printf("Situação: %s\n",
               conta.situacao == CONTA_ATIVA ? "ATIVA" : "ENCERRADA");
    }
    else{
        printf("Conta não encontrada.\n");
    }
    fclose(arquivo);
}

int depositar(long numeroConta, double valor){
    FILE *arquivo;
    Conta conta;

    if (valor <= 0){
        printf("Valor invalido.\n");
        return 0;
    }

    arquivo = fopen(ARQUIVO, "rb+");
    if (arquivo == NULL){
        printf("Nenhuma conta cadastrada ainda. \n");
        return 0;
    }

    if (!buscarConta(arquivo, numeroConta, &conta)){
        printf("Conta não encontrada.\n");
        fclose(arquivo);
        return 0;
    }

    if (conta.situacao != CONTA_ATIVA){
        printf("Conta encerrada.\n");
        fclose(arquivo);
        return 0;
    }

    conta.saldo += valor;

    if (!atualizarConta(arquivo, &conta)){
        printf("Erro ao atualizar a conta.\n");
        fclose(arquivo);
        return 0;
    }
    fclose(arquivo);
    return 1;
}

int sacar(long numeroConta, double valor){
    FILE *arquivo;
    Conta conta;

    if (valor <= 0){
        printf("Valor inválido.\n");
        return 0;
    }

    arquivo = fopen(ARQUIVO, "rb+");
    if (arquivo == NULL){
        printf("Nenhuma conta cadastrada ainda.\n");
        return 0;
    }

    if (!buscarConta(arquivo, numeroConta, &conta)){
        printf("Conta não encontrada.\n");
        fclose(arquivo);
        return 0;
    }

    if (conta.situacao != CONTA_ATIVA){
        printf("Conta encerrada.\n");
        fclose(arquivo);
        return 0;
    }

    if (conta.saldo < valor){
        printf("Saldo insuficiente.\n");
        fclose(arquivo);
        return 0;
    }

    conta.saldo -= valor;

    if (!atualizarConta(arquivo, &conta)){
        printf("Erro ao atualizar a conta.\n");
        fclose(arquivo);
        return 0;
    }
    fclose(arquivo);
    return 1;
}

int transferir(long contaOrigem, long contaDestino, double valor){
    FILE *arquivo;
    Conta origem, destino;

    if (valor <= 0){
        printf("Valor inválido.\n");
        return 0;
    }

    if (contaOrigem == contaDestino){
        printf("Origem e destino devem ser diferentes.\n");
        return 0;
    }

    arquivo = fopen(ARQUIVO, "rb");
    if (arquivo == NULL){
        printf("Nenhuma conta cadastrada ainda.\n");
        return 0;
    }

    if (!buscarConta(arquivo, contaOrigem, &origem) ||
        !buscarConta(arquivo, contaDestino, &destino)){
        printf("Conta de origem ou destino não encontrada.\n");
        fclose(arquivo);
        return 0;
    }
    fclose(arquivo);

    if (origem.situacao != CONTA_ATIVA || destino.situacao != CONTA_ATIVA){
        printf("Uma das contas está encerrada.\n");
        return 0;
    }

    if (origem.saldo < valor){
        printf("Saldo insuficiente.\n");
        return 0;
    }

    if (!sacar(contaOrigem, valor))
        return 0;
    
    if (!depositar(contaDestino, valor)){
        depositar(contaOrigem, valor);
        printf("Transferência cancelada. Valor devolvido.\n");
        return 0;
    }
    return 1;
}

void listarContas(void){
    FILE *arquivo;
    Conta conta;
    int filtro;
    int total = 0;

    arquivo = fopen(ARQUIVO, "rb");
    if (arquivo == NULL){
        printf("Nenhuma conta cadastrada ainda.\n");
        return;
    }

    printf("1 - Todas\n");
    printf("2 - Somente ativas\n");
    printf("3 - Somente encerradas\n");
    printf("Escolha uma opção: ");
    scanf("%d", &filtro);
    getchar();

    if (filtro < 1 || filtro > 3){
        printf("Opção inválida.\n");
        fclose(arquivo);
        return;
    }

    printf("\n--------Lista de contas--------\n");

    while (fread(&conta, sizeof(Conta), 1, arquivo) == 1){
        if (filtro == 2 && conta.situacao != CONTA_ATIVA)
            continue;
        if (filtro == 3 && conta.situacao != CONTA_ENCERRADA)
            continue;

        printf("Conta %ld | %s | CPF %s | R$ %.2f | %s\n",
               conta.numero,
               conta.titular,
               conta.cpf,
               conta.saldo,
               conta.situacao == CONTA_ATIVA ? "ATIVA" : "ENCERRADA");
        total++;
    }

    if (total == 0)
        printf("Nenhuma conta encontrada.\n");
    
    fclose(arquivo);
}

int encerrarConta(long numeroConta){
    FILE *arquivo;
    Conta conta;
    int ok = 0;

    arquivo = fopen(ARQUIVO, "rb+");
    if (arquivo == NULL){
        printf("Nenhuma conta cadastrada ainda.\n");
        return 0;
    }

    if (!buscarConta(arquivo, numeroConta, &conta)){
        printf("Conta não encontrada.\n");
    } else if (conta.situacao == CONTA_ENCERRADA){
        printf("A conta já está encerrada.\n");
    } else if (conta.saldo >= 0.005 || conta.saldo <= -0.005){  
        printf("Para encerrar, o saldo precisa ser zero.\n");
    } else{
        conta.situacao = CONTA_ENCERRADA;
        ok = atualizarConta(arquivo, &conta);
        if (!ok)
            printf("Erro ao atualizar a conta.\n");
    }
    fclose(arquivo);
    return ok;
}

void extrato(void){
    FILE *arquivo;
    Transacao t;
    long numero;
    int total = 0;
    const char *nomes[] = {"Deposito", "Saque",
                           "Transferencia enviada",
                           "Transferencia recebida"};

    printf("Número da conta: ");
    scanf("%ld", &numero);
    getchar();

    arquivo = fopen(ARQUIVO_TRANS, "rb");
    if (arquivo == NULL){
        printf("Nenhuma transacao registrada ainda.\n");
        return;
    }

    printf("\n--------Extrato da conta %ld--------\n", numero);

    while (fread(&t, sizeof(Transacao), 1, arquivo) == 1){
        if (t.conta != numero)
            continue;

        printf("%-24s R$ %.2f", nomes[t.tipo], t.valor);
        if (t.tipo == TRANSFERENCIA_ENVIADA)
            printf(" (para a conta %ld)", t.contaRelacionada);
        else if (t.tipo == TRANSFERENCIA_RECEBIDA)
            printf(" (da conta %ld)", t.contaRelacionada);
        printf("\n");
        total++;
    }

    if (total == 0)
        printf("Nenhuma transacao encontrada.\n");

    fclose(arquivo);
}

int reativarConta(long numeroConta){
    FILE *arquivo;
    Conta conta;
    int ok = 0;

    arquivo = fopen(ARQUIVO, "rb+");
    if (arquivo == NULL){
        printf("Nenhuma conta cadastrada ainda.\n");
        return 0;
    }

    if (!buscarConta(arquivo, numeroConta, &conta)){
        printf("Conta não encontrada.\n");
    } else if (conta.situacao == CONTA_ATIVA){
        printf("A conta já está ativa.\n");
    } else{
        conta.situacao = CONTA_ATIVA;
        ok = atualizarConta(arquivo, &conta);
        if (!ok)
            printf("Erro ao atualizar a conta.\n");
    }
    fclose(arquivo);
    return ok;
}

#define ARQUIVO_TMP "contas.tmp"

int removerEncerradas(void){
    FILE *origem, *temp;
    Conta conta;
    int removidas = 0;

    origem = fopen(ARQUIVO, "rb");
    if (origem == NULL){
        printf("Nenhuma conta cadastrada ainda.\n");
        return -1;
    }

    temp = fopen(ARQUIVO_TMP, "wb");
    if (temp == NULL){
        printf("Erro ao criar o arquivo temporario.\n");
        fclose(origem);
        return -1;
    }

    while (fread(&conta, sizeof(Conta), 1, origem) == 1){
        if (conta.situacao == CONTA_ATIVA){
            if (fwrite(&conta, sizeof(Conta), 1, temp) != 1){
                printf("Erro ao gravar o arquivo temporario.\n");
                fclose(origem);
                fclose(temp);
                remove(ARQUIVO_TMP);
                return -1;
            }
        } else{
            removidas++;
        }
    }

    fclose(origem);
    fclose(temp);

    if (remove(ARQUIVO) != 0 || rename(ARQUIVO_TMP, ARQUIVO) != 0){
        printf("Erro ao substituir o arquivo.\n");
        return -1;
    }

    return removidas;
}

int main(void){
    int opcao;
    long numero, destino;
    double valor;

    do{
        printf("\n===== CAIXA ELETRÔNICO =====\n");
        printf("\n1 - Cadastrar conta\n");
        printf("\n2 - Consultar conta\n");
        printf("\n3 - Realizar depósito\n");
        printf("\n4 - Realizar saque\n");
        printf("\n5 - Realizar transferência\n");
        printf("\n6 - Listar contas\n");
        printf("\n7 - Encerrar conta\n");
        printf("\n8 - Extrato\n");
        printf("\n9 - Reativar conta\n");
        printf("\n10 - Remover contas encerradas\n");
        printf("\n0 - Sair\n");
        printf("\nOpcao: \n");

        if (scanf("%d", &opcao) !=1){
            while (getchar() != '\n');
            opcao = -1;
        }
        else
            getchar();

        switch (opcao){
            case 1:
                cadastrarConta();
                break;
            case 2:
                consultarConta();
                break;
            case 3:
                printf("Número da conta: ");
                scanf("%ld", &numero);
                printf("Valor do depósito: ");
                scanf("%lf", &valor);
                getchar();
                if (depositar(numero, valor)){
                   registrarTransacao(numero, 0, DEPOSITO, valor);
                   printf("Depósito realizado com sucesso!\n");
                }
                break;
            case 4:
                printf("Número da conta: ");
                scanf("%ld", &numero);
                printf("Valor do saque: ");
                scanf("%lf", &valor);
                getchar();
                if (sacar(numero, valor)){
                   registrarTransacao(numero, 0, SAQUE, valor);
                   printf("Saque realizado com sucesso!\n");
                }
                break;
            case 5:
                printf("Conta de origem: ");
                scanf("%ld", &numero);
                printf("Conta de destino: ");
                scanf("%ld", &destino);
                printf("Valor da transferência: ");
                scanf("%lf", &valor);
                getchar();
                if (transferir(numero, destino, valor)){
                   registrarTransacao(numero, destino, TRANSFERENCIA_ENVIADA, valor);
                   registrarTransacao(destino, numero, TRANSFERENCIA_RECEBIDA, valor);
                   printf("Transferência realizada com sucesso!\n");
                }
                break;
            case 6:
                listarContas();
                break;
            case 7:
                printf("Número da conta: ");
                scanf("%ld", &numero);
                getchar();
                if (encerrarConta(numero))
                   printf("Conta encerrada com sucesso!\n");
                break;
            case 8:
                extrato();
                break;
            case 9:
                printf("Número da conta: ");
                scanf("%ld", &numero);
                getchar();
                if (reativarConta(numero))
                    printf("Conta reativada com sucesso!\n");
                break;
            case 10:{
                char conf;
                printf("Isso apaga DEFINITIVAMENTE as contas encerradas. Confirmar (s/n)? ");
                scanf(" %c", &conf);
                getchar();
                if (conf == 's' || conf == 'S'){
                    int qtd = removerEncerradas();
                    if (qtd >= 0)
                        printf("%d conta(s) removida(s).\n", qtd);
                }
                break;
            }
            case 0:
                printf("Saindo...\n");
                break;
            default:
                printf("Opção inválida.\n");
        }
    } while (opcao !=0);

    return 0;
}