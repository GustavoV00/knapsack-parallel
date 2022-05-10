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
        self.tempo = {
            "tp": {
                "1": {},
                "2": {},
                "3": {},
                "4": {},
            },
            "tpSerial": {
                "1": {},
                "2": {},
                "3": {},
                "4": {},
            },
            "tpSerialMedio": [],
            "tpSerialDesvioPadrao": [],
            "tpMedio": {
                "1": [],
                "2": [],
                "3": [],
                "4": [],
            },
            "tpDesvioPadrao": {
                "1": [],
                "2": [],
                "3": [],
                "4": [],
            }
        }

        self.speedup = {
            "sp": {
                "1": {},
                "2": {},
                "3": {},
                "4": {},
            },
            "spMedio": {
                "1": [],
                "2": [],
                "3": [],
                "4": [],

            },
            "spDesvioPadrao": {
                "1": [],
                "2": [],
                "3": [],
                "4": [],
            }
        }

        self.amdahl = []

        self.eficiencia = {
            "ef": {
                "1": {},
                "2": {},
                "3": {},
                "4": {},
            },
            "efMedio": {
                "1": [],
                "2": [],
                "3": [],
                "4": [],

            },
            "efDesvioPadrao": {
                "1": [],
                "2": [],
                "3": [],
                "4": [],
            }
        }

    def executaMochila(self, proc, testes):
        i = 0
        for elem in testes:
            command = f"mpirun --np {proc} kn < {elem} >> ./testes/testRelatorio/out/c{proc}/saida_mochila_{i}.out"
            print(command)
            for _ in range(20):
                os.system(command)
            time.sleep(0.2)
            i += 1

    def leTempo(self, elem):
        result = []
        result2 = []
        with open(elem) as file:
            lines = [line.rstrip() for line in file]
        for elem in lines:
            x = elem.split()
            print(x)
            if(x[1] == "time:"):
                result.append(float(x[1]))
            if(x[0] == "time:"):
                result.append(float(x[1]))
            if(x[0] == "serialTime:"):
                result2.append(float(x[1]))
            if(x[1] == "serialTime:"):
                result2.append(float(x[1]))

        return [result, result2]

    def mediaEDesvioPadrao(self, tempo):
        # print(tempo, type(tempo))
        media = np.average(tempo)
        desvioPadrao = np.std(tempo)
        return [round(media, 4), round(desvioPadrao, 4)]

    def obterSpeedUp(self, seq, paral):
        result = []
        for i in range(len(paral)):
            x = seq[i] / paral[i]
            result.append(round(x, 4))
        return result

    def obterEficiencia(self, speedup, proc):
        result = []
        for elem in speedup:
            x = elem / proc
            result.append(round(x, 4))

        return result

    def obterAmdahl(self, processors, mediaSeqTime):
        # seqPorcent = (mediaSeqTime * 100) / mediaTotalTime
        # seqPorcent = seqPorcent / 100

        amdal = 1 / ((1 - (mediaSeqTime)) / processors) + mediaSeqTime
        return round(amdal, 4)

    def calculaPorcent(self, lista, totalTime):
        result = []
        for i in lista: 
            seqPorcent = (i * 100) / totalTime
            seqPorcent = seqPorcent / 100
            result.append(seqPorcent)

        return result
    

def main():

    path = "./testes/testRelatorio/in"
    testes = testes = [f"{path}/teste7.in"]
    c1 = Data(1)
    # c2 = DataPl(2)
    # c3 = DataPl(3)
    # c4 = DataPl(4)
    procs = [2, 3, 4]
    for i in range(len(testes)):
        compilaTudo()
        saida = [f"./testes/testRelatorio/out/c{1}/saida_mochila_{j}.out" for j in range(len(testes))]
        c1.executaMochila(1, testes)
        [result, result2] = c1.leTempo(saida[i])
        print(result, result2)

        c1.tempo["tp"][f"{c1.proc}"].update({f"teste{i+1}": result})
        c1.tempo["tpSerial"][f"{c1.proc}"].update({f"teste{i+1}": result2})

        tempo = c1.tempo["tp"][f"{c1.proc}"][f"teste{i+1}"]
        [media, dv] = c1.mediaEDesvioPadrao(tempo)
        c1.tempo["tpMedio"][f"{c1.proc}"].append(media)
        c1.tempo["tpDesvioPadrao"][f"{c1.proc}"].append(dv)

        speedUp = c1.obterSpeedUp(tempo, tempo)
        c1.speedup["sp"][f"{c1.proc}"].update({f"speedup{i+1}": speedUp})

        [mediaSp, dvSp] = c1.mediaEDesvioPadrao(speedUp)
        c1.speedup["spMedio"][f"{c1.proc}"].append(mediaSp)
        c1.speedup["spDesvioPadrao"][f"{c1.proc}"].append(dvSp)

        eficiencia = c1.obterEficiencia(speedUp, 1)
        c1.eficiencia["ef"][f"{1}"].update({f"eficiencia{i+1}": eficiencia})

        [mediaEf, dvEf] = c1.mediaEDesvioPadrao(eficiencia)
        c1.eficiencia["efMedio"][f"{1}"].append(mediaEf)
        c1.eficiencia["efDesvioPadrao"][f"{1}"].append(dvEf)
        

        for proc in procs:
            compilaTudo()
            saida = [f"./testes/testRelatorio/out/c{proc}/saida_mochila_{j}.out" for j in range(len(testes))]
            c1.executaMochila(proc, testes)

            k = i
            [result, result2] = c1.leTempo(saida[i])
            c1.tempo["tp"][f"{proc}"].update({f"teste{k+1}": result})
            c1.tempo["tpSerial"][f"{proc}"].update({f"teste{k+1}": result2})
            tempo2 = c1.tempo["tp"][f"{proc}"][f"teste{k+1}"]

            [media, dv] = c1.mediaEDesvioPadrao(tempo2)
            c1.tempo["tpMedio"][f"{proc}"].append(media)
            c1.tempo["tpDesvioPadrao"][f"{proc}"].append(dv)

            speedUp = c1.obterSpeedUp(tempo, tempo2)
            c1.speedup["sp"][f"{proc}"].update({f"speedup{k+1}": speedUp})

            [mediaSp, dvSp] = c1.mediaEDesvioPadrao(speedUp)
            c1.speedup["spMedio"][f"{proc}"].append(mediaSp)
            c1.speedup["spDesvioPadrao"][f"{proc}"].append(dvSp)

            eficiencia = c1.obterEficiencia(speedUp, proc)
            c1.eficiencia["ef"][f"{proc}"].update({f"eficiencia{k+1}": eficiencia})

            [mediaEf, dvEf] = c1.mediaEDesvioPadrao(eficiencia)
            c1.eficiencia["efMedio"][f"{proc}"].append(mediaEf)
            c1.eficiencia["efDesvioPadrao"][f"{proc}"].append(dvEf)

            seq = c1.tempo["tpSerial"][f"{proc}"][f"teste{k+1}"]
            totalTime = c1.tempo["tpMedio"][f"{proc}"][k]
            seqResult = c1.calculaPorcent(seq, totalTime)
            mediaSeqTime = np.average(seqResult)
            print("MEDIASEQTIME: ", mediaSeqTime)

            c1.tempo["tpSerialMedio"].append(mediaSeqTime)

            dvSeqTime = np.std(seqResult)
            c1.tempo["tpSerialDesvioPadrao"].append(dvSeqTime)

            amdahl = c1.obterAmdahl(c1.proc, mediaSeqTime)
            c1.amdahl.append(amdahl)
            k += 1

    print("TempoMedio: ", c1.tempo["tpMedio"], "\n\n\n\n\n")
    print("DesvioPadrao: ", c1.tempo["tpDesvioPadrao"], "\n\n\n\n\n")

    print("MedioSerial: ", c1.tempo["tpSerialMedio"], "\n\n\n\n\n")
    print("DesvioPadraoSerial: ", c1.tempo["tpSerialDesvioPadrao"], "\n\n\n\n\n")

    print("speedupMedio: ", c1.speedup["spMedio"], "\n\n\n\n\n")
    print("speedupDsvioPadrao: ", c1.speedup["spDesvioPadrao"], "\n\n\n\n\n")

    print("eficiencia media: ", c1.eficiencia["efMedio"], "\n\n\n\n\n")
    print("eficiecniaDesvioPadrao: ", c1.eficiencia["efDesvioPadrao"], "\n\n\n\n\n")

    print(c1.amdahl, "\n\n\n\n\n")




if __name__ == "__main__":
    main()
