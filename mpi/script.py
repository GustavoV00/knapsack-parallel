import os
import numpy as np
import time

def compilaTudo():
    try:
        os.system("make clean")
        os.system("make")
        os.system("rm ./testes/testRelatorio/out/c1/*")
        os.system("rm ./testes/testRelatorio/out/c2/*")
        os.system("rm ./testes/testRelatorio/out/c3/*")
        os.system("rm ./testes/testRelatorio/out/c4/*")
    except:
        os.system("make")

class Data:
    def __init__(self, proc):
        self.proc = proc
        self.tempoMedio = []
        self.desvioPadraoTempoMedio = []

        self.speedup = []
        self.speedUpMedia = []
        self.speedUpDesvioPadrao = []

        self.testes = testes = ["./testes/testRelatorio/in/teste1.in", "./testes/testRelatorio/in/teste2.in"]
        self.saida = [f"./testes/testRelatorio/out/c{self.proc}/saida_mochila_{i}.out" for i in range(len(testes))]



    def executaMochila(self):
        i = 0
        for elem in self.testes:
            command = f"mpirun --np {self.proc} kn < {elem} >> ./testes/testRelatorio/out/c{self.proc}/saida_mochila_{i}.out"
            print(command)
            for _ in range(20):
                os.system(command)
            time.sleep(0.2)
            i += 1

    def leTempo(self, tempo, elem):
        with open(elem) as file:
            lines = [line.rstrip() for line in file]
        for elem in lines:
            x = elem.split()
            if(x[0] == "time:"):
                tempo.append(float(x[1]))

        return tempo

    def mediaEDesvioPadrao(self, tempo):
        # print(tempo, type(tempo))
        media = np.average(tempo)
        desvioPadrao = np.std(tempo, dtype=float)
        return [media, desvioPadrao]

    def obterSpeedUp(self, seq, paral):
        for i in range(len(paral)):
            result = seq[i] / paral[i]
            self.speedup.append(result)

class DataSeq(Data):
    def __init__(self, proc):
        super().__init__(proc)
        self.tempoSequencial = []

    def imprimeelementos(self):
        print("proc: ", self.proc)
        print("tempoMedio: ", self.tempoMedio)
        print("tempoDesvioPadrao: ", self.desvioPadraoTempoMedio)

        print("SpeedUp: ", self.speedup)
        print("speedUpMedia: ", self.speedUpMedia)
        print("speedUpDesvioPadrao: ", self.desvioPadraoTempoMedio)

class DataPl(Data):
    def __init__(self, proc):
        super().__init__(proc)
        self.tempoParalelo = []
        self.porcentagemTempoSequencial = []
        self.porcentagemTempoParalelo = []
        self.tempoTotal = []
        self.amdahl = []


        self.eficiencia = []
        self.eficienciaMedia = []
        self.eficienciaDesvioPadrao = []

    def obterSpeedUp(self, seq, paral):
        for i in range(len(paral)):
            result = seq[i] / paral[i]
            self.speedup.append(result)

    def obterEficiencia(self, speedup, proc):
        for elem in speedup:
            result = elem / proc
            self.eficiencia.append(result)

    def obterAmdahl(self, processors):
        # seqPorcent = (media_seqParalTime * 100) / media_totalTime
        # seqPorcent = seqPorcent / 100

        amdal = 1 / ((1 - (0)) / processors) + 0
        return amdal

    def imprimeelementos(self):
        print("proc: ", self.proc)
        print("tempoMedio: ", self.tempoMedio)
        print("tempoDesvioPadrao: ", self.desvioPadraoTempoMedio)

        print("SpeedUp: ", self.speedup)
        print("speedUpMedia: ", self.speedUpMedia)
        print("speedUpDesvioPadrao: ", self.desvioPadraoTempoMedio)

        print("Eficiencia: ", self.eficiencia)
        print("EficienciaMedia: ", self.eficienciaMedia)
        print("EficienciaDesvioPadrao: ", self.eficienciaDesvioPadrao)

        print("amdal: ", self.amdahl)

def main():

    c1 = DataSeq(1)
    c2 = DataPl(2)
    c3 = DataPl(3)
    c4 = DataPl(4)
    procs = [c2, c3, c4]
    for elem in c1.saida:
        compilaTudo()
        c1.executaMochila()
        c1.tempoSequencial = c1.leTempo(c1.tempoSequencial, elem)
        [media, dv] = c1.mediaEDesvioPadrao(c1.tempoSequencial)
        c1.tempoMedio.append(media)
        c1.desvioPadraoTempoMedio.append(dv)

        c1.obterSpeedUp(c1.tempoSequencial, c1.tempoSequencial)
        [mediaSp, dvSp] = c1.mediaEDesvioPadrao(c1.speedup)
        c1.speedUpMedia.append(mediaSp)
        c1.speedUpDesvioPadrao.append(dvSp)
        for c in procs:
            c.executaMochila()
            for elem in c.saida:
                c.tempoParalelo = c.leTempo(c.tempoParalelo, elem)

                [media, dv] = c.mediaEDesvioPadrao(c.tempoParalelo)
                c.tempoMedio.append(media)
                c.desvioPadraoTempoMedio.append(media)

                c.obterSpeedUp(c1.tempoSequencial, c.tempoParalelo)
                [mediaSp, dvSp] = c.mediaEDesvioPadrao(c.speedup)
                c.speedUpMedia.append(mediaSp)
                c.speedUpDesvioPadrao.append(dvSp)

                c.obterEficiencia(c.speedup, c.proc)
                [mediaEf, dvEf] = c.mediaEDesvioPadrao(c.eficiencia)
                c.speedUpMedia.append(mediaEf)
                c.speedUpDesvioPadrao.append(dvEf)

                c.obterAmdahl(c.proc)
                c.tempoParalelo.clear()
        c1.tempoSequencial.clear()


    c1.imprimeelementos()
    print("\n\n\n\n")

    c2.imprimeelementos()
    print("\n\n\n\n")

    c3.imprimeelementos()
    print("\n\n\n\n")

    c4.imprimeelementos()


if __name__ == "__main__":
    main()
