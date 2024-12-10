#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include "globals.h"



#define MEM_SIZE 16384 // memoria de 16kb 


int8_t mem[MEM_SIZE];
uint32_t pc = 0x00000000; // contador
uint32_t ri = 0x00000000; // registrador de instrucao
int32_t registers[32]; // banco de registradores - x0 a x31 //ESSE TIPO ESTAVA ERRADO! ESTAVA uint32_t PORÉM UM REGISTRADOR PODE CONTER VALORES NEGATIVOS.

// instrucoes
uint32_t opcode, rs1, rs2, rd, shamnt, funct3, funct7;
int32_t imm12_i, imm12_s, imm13, imm20_u, imm21;



//acesso a memoria funcoes tiradas do trabalho 1

void update_pc(uint32_t target) {
    if (target < MEM_SIZE) {
        pc = target;
    } else {
        printf("Erro: Tentativa de salto fora dos limites da memória (PC=0x%08x).\n", target);
        exit(1); // Finaliza o simulador em caso de erro
    }
}

// leitura de byte com sinal
int32_t lb(int32_t reg, int32_t kte) {
    int32_t address = reg + kte;
    if (address < 0 || address >= MEM_SIZE) {
        printf("erro: endereco fora dos limites de memoria\n");
        return -1;
    }
    int8_t byte = mem[address];
    return (int32_t) byte;
}

//O retorno aqui estava errado. Você estava retornando um int32_t (número com sinal).
// leitura de byte sem sinal
uint32_t lbu(int32_t reg, int32_t kte) {
    int32_t address = reg + kte;  // calcula o endereco somando o valor do registrador e o deslocamento
    if (address < 0 || address >= MEM_SIZE) { // verifica se o endereco esta dentro dos limites
        printf("erro: endereco fora dos limites de memoria\n");
        return -1;
    }
    uint8_t byte = (uint8_t) mem[address];  // le o byte da memoria e o interpreta como sem sinal
    return (uint32_t) byte;  // converte o byte lido para int32_t sem estender o sinal
}

// leitura de palavra (32 bits)
int32_t lw(int32_t reg, int32_t kte) {
    int32_t address = reg + kte;
    if (address % 4 != 0) {
        printf("erro: endereco nao e multiplo de 4\n");
        return -1;  // se nao for multiplo de 4 = endereco invalido
    }
    int32_t word = ((uint8_t)mem[address + 3] << 24) |
                   ((uint8_t)mem[address + 2] << 16) |
                   ((uint8_t)mem[address + 1] << 8) |
                   ((uint8_t)mem[address]);
    return word;
}

// escrita de byte na memoria
void sb(int32_t reg, int32_t kte, int8_t byte) {
    int32_t address = reg + kte;  // calcula o endereco somando o registrador e o deslocamento
    if (address < 0 || address >= MEM_SIZE) { // verifica se o endereco esta dentro dos limites
        printf("erro: endereco fora dos limites de memoria\n");
        return;
    }
    mem[address] = byte;  // armazena o byte no endereco calculado
}

// escrita de palavra (32 bits) na memoria
void sw(int32_t reg, int32_t kte, int32_t word) {
    int32_t address = reg + kte;
    if (address % 4 != 0) {
        printf("erro: endereco nao e multiplo de 4\n");
        return;  // se nao for multiplo de 4 = endereco invalido
    }
    mem[address] = word & 0xFF; // lsb
    mem[address + 1] = (word >> 8) & 0xFF;
    mem[address + 2] = (word >> 16) & 0xFF;
    mem[address + 3] = (word >> 24) & 0xFF; // msb
}



// funcao de busca

void fetch() {
    
    //printf("Buscando instrução no PC = 0x%08X\n", pc);  // Adicionando print para ver o valor do PC
    ri = lw(pc, 0); // busca a instrução na memória
    //printf("Instrução carregada: 0x%08X\n", ri);  // Imprime a instrução
    pc += 4; // incrementa o contador de programa
}


//funcao de decodificacao

/*void decode() {
    opcode = ri & 0x7F; // pega os 7 bits menos significativos
    rd = (ri >> 7) & 0x1F; // pega bits 7 a 11
    funct3 = (ri >> 12) & 0x7; // pega bits 12 a 14
    rs1 = (ri >> 15) & 0x1F; // pega bits 15 a 19
    rs2 = (ri >> 20) & 0x1F; // pega bits 20 a 24
    shamnt = (ri >> 20) & 0x1F; // pega bits 20 a 24
    funct7 = (ri >> 25) & 0x7F; // pega bits 25 a 31


    imm12_i = (int32_t)(ri >> 20) << 20 >> 20;
    imm12_s = ((int32_t)(ri >> 25) << 5 | ((ri >> 7) & 0x1F)) << 20 >> 20; // Tipo S, sinal estendido
    imm13 = ((ri >> 31) << 12) | (((ri >> 7) & 1) << 11) | (((ri >> 25) & 0x3F) << 5) | ((ri >> 8) & 0xF) << 1; // Tipo SB, sinal estendido
    imm20_u = ri & 0xFFFFF000;           // Tipo U (LUI, AUIPC)
    imm21 = ((ri >> 31) << 20) | (((ri >> 12) & 0xFF) << 12) | (((ri >> 20) & 1) << 11) | ((ri >> 21) & 0x3FF) << 1; // Tipo UJ, sinal estendido
}*/



void decode () {
    int32_t tmp;
    opcode	= ri & 0x7F;
    rs2		= (ri >> 20) & 0x1F;
    rs1		= (ri >> 15) & 0x1F;
    rd		= (ri >> 7)  & 0x1F;
    shamnt	= (ri >> 20) & 0x1F;
    funct3	= (ri >> 12) & 0x7;
    funct7  = (ri >> 25);
    imm12_i = ((int32_t)ri) >> 20;
    tmp     = get_field(ri, 7, 0x1f);
    imm12_s = set_field(imm12_i, 0, 0x1f, tmp);
    imm13   = imm12_s;
    imm13 = set_bit(imm13, 11, imm12_s&1);
    imm13 = imm13 & ~1;
    imm20_u = ri & (~0xFFF);
    // mais aborrecido...
    imm21 = (int32_t)ri >> 11;			// estende sinal
    tmp = get_field(ri, 12, 0xFF);		// le campo 19:12
    imm21 = set_field(imm21, 12, 0xFF, tmp);	// escreve campo em imm21
    tmp = get_bit(ri, 20);				// le o bit 11 em ri(20)
    imm21 = set_bit(imm21, 11, tmp);			// posiciona bit 11
    tmp = get_field(ri, 21, 0x3FF);
    imm21 = set_field(imm21, 1, 0x3FF, tmp);
    imm21 = imm21 & ~1;					// zero bit 0
}

//funcao de execucao

void execute() {
    //printf("Executando a instrução com opcode: 0x%02X\n", opcode);
    registers[0] = 0x00000000; // Colocado aqui para que sempre o "execute" seja executado com o registrador 0 com o valor constante 0.  
    switch (opcode) {
        // tipo r (instrucoes aritmeticas e logicas entre registradores)
        case 0x33: // tipo r (add, sub, sll, srl, sra, and, or, xor, slt, sltu)
            //printf("Instrução tipo R\n");
            switch (funct3) {
                //printf("Valores de a0 = %d, a1 = %d\n", registers[10], registers[11]);

                case 0x0: // add ou sub
                    if (funct7 == 0x00) {
                        registers[rd] = registers[rs1] + registers[rs2]; // add
                    } else if (funct7 == 0x20) {
                        registers[rd] = registers[rs1] - registers[rs2]; // sub
                    }
                    break;
                case 0x1: // sll (shift logico a esquerda)
                    registers[rd] = registers[rs1] << (registers[rs2] & 0x1F);
                    break;
                case 0x2: // slt (menor que com sinal)
                    registers[rd] = (registers[rs1] < registers[rs2]) ? 1 : 0;
                    break;
                case 0x3: // sltu (menor que sem sinal)
                    registers[rd] = ((uint32_t)registers[rs1] < (uint32_t)registers[rs2]) ? 1 : 0;
                    break;
                case 0x4: // xor (ou exclusivo)
                    registers[rd] = registers[rs1] ^ registers[rs2];
                    break;
                case 0x5: // srl ou sra (shift a direita logico e aritmetico)
                    if (funct7 == 0x00) {
                        registers[rd] = (uint32_t)registers[rs1] >> (registers[rs2] & 0x1F); // srl
                    } else if (funct7 == 0x20) {
                        registers[rd] = registers[rs1] >> (registers[rs2] & 0x1F); // sra
                    }
                    break;
                case 0x6: // or
                    registers[rd] = registers[rs1] | registers[rs2];
                    break;
                case 0x7: // and
                    registers[rd] = registers[rs1] & registers[rs2];
                    break;
            }
            break;

        // tipo i (instrucoes aritmeticas/logicas com imediato e carregamento de memoria)
        case 0x13: // tipo i (addi, slti, sltiu, xori, ori, andi, slli, srli, srai)
           // printf("Valores de a0 = %d, a1 = %d\n", registers[10], registers[11]);

            //printf("Instrução tipo R\n");
            switch (funct3) {
                case 0x0: // addi
                    registers[rd] = registers[rs1] + imm12_i;    
                    break;
                case 0x2: // slti
                    registers[rd] = (registers[rs1] < imm12_i) ? 1 : 0;
                    break;
                case 0x3: // sltiu
                    registers[rd] = ((uint32_t)registers[rs1] < (uint32_t)imm12_i) ? 1 : 0;
                    break;
                case 0x4: // xori
                    registers[rd] = registers[rs1] ^ imm12_i;
                    break;
                case 0x6: // ori
                    registers[rd] = registers[rs1] | imm12_i;
                    break;
                case 0x7: // andi
                    registers[rd] = registers[rs1] & imm12_i;
                    break;
                case 0x1: // slli (shift logico a esquerda com imediato)
                    registers[rd] = registers[rs1] << shamnt;
                    break;
                case 0x5:
                    if (funct7 == 0x00) {
                        registers[rd] = (uint32_t)registers[rs1] >> shamnt; // srli
                    } else if (funct7 == 0x20) {
                        registers[rd] = registers[rs1] >> shamnt; // srai
                    }
                    break;
            }
            break;

        case 0x03: // tipo i - carregamento de memoria (lb, lbu, lw)
           // printf("Valores de a0 = %d, a1 = %d\n", registers[10], registers[11]);

            switch (funct3) {
                case 0x0: // lb (carregar byte com sinal) Anteriormente estava registers[rd] = lb(registers[rs1] - 4, imm12_i), não existe a nessecidade de subtrair 4. Você fez isso em todas as instruções com esse opcode.
                    registers[rd] = lb(registers[rs1], imm12_i);
                    break;
                case 0x4: // lbu (carregar byte sem sinal)
                    registers[rd] = lbu(registers[rs1], imm12_i);
                    break;
                case 0x2: // lw (carregar palavra)
                    registers[rd] = lw(registers[rs1], imm12_i);
                    break;
            }
            break;

        // tipo s (armazenamento de memoria)
        case 0x23: // tipo s (sb, sw)
            switch (funct3) {
                case 0x0: // sb (armazenar byte) Corrigido (subtração de 4 sem nessecidade).
                    sb(registers[rs1], imm12_s, registers[rs2] & 0xFF);
                    break;
                case 0x2: // sw (armazenar palavra)
                    sw(registers[rs1], imm12_s, registers[rs2]);
                    break;
            }
            break;

        // tipo sb (desvio condicional)
        case 0x63: // tipo sb (beq, bne, blt, bge, bltu, bgeu)
            switch (funct3) {
                case 0x0: // beq (igual)
                
                    if (registers[rs1] == registers[rs2]) {
                            
                            //update_pc(pc + imm13 - 4);
                            pc = (pc - 4) + imm13;
                        }

                    
                    break;
                case 0x1: // bne (diferente)
                    if (registers[rs1] != registers[rs2]) pc += (imm13 - 4);
                    break;
                case 0x4: // blt (menor que)
                    if (registers[rs1] < registers[rs2]) pc += (imm13 - 4);
                    break;
                case 0x5: // bge (maior ou igual)
                    if (registers[rs1] >= registers[rs2]) pc += (imm13 - 4);
                    break;
                case 0x6: // bltu (menor que, sem sinal)
                    if ((uint32_t)registers[rs1] < (uint32_t)registers[rs2]) pc += (imm13 - 4);
                    break;
                case 0x7: // bgeu (maior ou igual, sem sinal)
                    if ((uint32_t)registers[rs1] >= (uint32_t)registers[rs2]) pc += (imm13 - 4);
                    break;
            }
            break;

        // tipo u (lui, auipc)
        case 0x37: // lui (load upper immediate)
            registers[rd] = imm20_u;
            break;

        case 0x17: // auipc (add upper immediate to pc) 
            registers[rd] = (pc - 4) + imm20_u;
            break;

        // tipo uj (jal, salto incondicional)

        case 0x6F: // jal (jump and link)
            //printf("Jal %d\n", pc);
            registers[rd] = pc;
            pc = pc - 4;
            pc += imm21;
            break;

        case 0x67: // jalr (jump and link register, tipo i com funct3 = 0x0)
            registers[rd] = pc; 
            pc = (registers[rs1] + imm12_i); // JALR mudado
            break;

        // ecall (chamada de sistema)

        case 0x73: // ecall
            switch(registers[17]){

            case 1:
            printf("%d", registers[10]);
            break;

            case 4:{
            uint32_t address = registers[10];
            while(mem[address] != '\0')
                 {
                    printf("%c", mem[address]);

                    address++;
                 }}
                     
            break;

            case 10:
            printf("Encerrando o simulador...\n");
            return;
            break;
            }
            break;          
    }

}


void step() {
    //printf("PC = 0x%08X\n", pc); 
    fetch();
    decode();
    execute();
}

void load_memory() {
    // Abrir os arquivos de código e dados
    FILE *code_f = fopen("code.bin", "rb");
    FILE *data_f = fopen("data.bin", "rb");

    // Verificar se os arquivos foram abertos corretamente
    if (code_f == NULL || data_f == NULL) {
        printf("erro ao abrir arquivos de memoria\n");
        exit(1);
    }

    // Verificar e carregar o código
    fseek(code_f, 0, SEEK_END);
    size_t code_size = ftell(code_f);  // Obtém o tamanho do arquivo de código
    fseek(code_f, 0, SEEK_SET);  // Volta o ponteiro de leitura para o início

    printf("Carregando c\xA2\digo\n");
    printf("C\xA2\digo carregado de %s com tamanho %u bytes\n", "code.bin", code_size);

    // Verificar se o código não é maior do que o espaço disponível
    if (code_size > 0x2000) { // Máximo de 8 KB para o segmento de código
        printf("arquivo de código muito grande\n");
        exit(1);
    }

    // Carregar o código na memória
    fread(&mem[0x0000], 1, code_size, code_f); // Carrega o código no endereço 0x00000000
    fclose(code_f); // Fecha o arquivo de código

    // Verificar e carregar os dados
    fseek(data_f, 0, SEEK_END);
    size_t data_size = ftell(data_f);  // Obtém o tamanho do arquivo de dados
    fseek(data_f, 0, SEEK_SET);  // Volta o ponteiro de leitura para o início

    printf("Carregando dados...\n");
    //printf("Dados carregados de %s com tamanho %u bytes\n", data_f, data_size);

    // Verificar se os dados não são maiores do que o espaço disponível
    if (data_size > (MEM_SIZE - 0x2000)) { // Máximo de 8 KB para o segmento de dados
        printf("arquivo de dados muito grande\n");
        exit(1);
    }

    // Carregar os dados na memória
    fread(&mem[0x2000], 1, data_size, data_f); // Carrega os dados no endereço 0x00002000
    fclose(data_f); // Fecha o arquivo de dados

    printf("Memória carregada com sucesso\n");
}


void run() {
             while (pc < MEM_SIZE && (opcode != 0x73 || registers[17] != 10))

                     step();
           }


int main() {
    registers[2] = 0x00003ffc; // Registrador sp 
    registers[3] = 0x00001800; // Registrador gp
    //if (argc < 3) {
      //  printf("uso: %s code.bin data.bin\n", argv[0]);
       // return 1;
    //}
    load_memory();
    printf("Executando o simulador...\n");
    run();
    printf("Execução concluida!\n");
    return 0;
}