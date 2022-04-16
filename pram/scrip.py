import os
import numpy as np

def main():
    i = 0
    flagSeq = 0
    fileName = "problems/result.out"
    fileName2 = "problems/result2.out"
    size = "151_30"
    os.system("make")

    while(i < 20):
        os.system(f"./knapsack-parallel < problems/{size} >> problems/result.out")
        if flagSeq == 1:
            os.system(f"./knapsack-serial  < problems/{size} >> problems/result2.out")
        i += 1

    resultado1 = 0
    media = 0
    if flagSeq == 1:
        results = []
        with open(fileName2, "r") as f:
            flag = 0
            for line in f:
                test = line.strip("\n")
                if(flag % 2 == 0):
                    results.append(float(test))
                else:
                    resultado1 = test
                flag += 1
            
            media = np.average(results)
            desvioPadrao = np.std(results)


            # print(results)
            print("-------------------------------------------------------------------")

            print("Media: %.5f" % media)
            print("desvioPadrao: %.5f" % desvioPadrao)
            print("\n")

            print("-------------------------------------------------------------------")



    seqParalTime = []
    paralTime = []
    totalTime = []
    resultado2 = 0
    with open(fileName, "r") as f:
        flag = 0
        for line in f:
            test = line.strip("\n")
            if(flag == 0):
                seqParalTime.append(float(test))
                flag += 1
            elif (flag == 1):
                paralTime.append(float(test))
                flag += 1
            elif(flag == 2):
                totalTime.append(float(test))
                flag += 1
            else:
                flag = 0
                resultado2 = test
        #         results.append(float(test))


    # print(seqParalTime)
    # print(paralTime)
    # print(totalTime)

    media_seqParalTime = np.average(seqParalTime)
    media_paralTime = np.average(paralTime)
    media_totalTime = np.average(totalTime)

    desvioPadrao_seqParalTime = np.std(seqParalTime)
    desvioPadrao_paralTime = np.std(paralTime)
    desvioPadrao_totalTime = np.std(totalTime)

    print("%.5f" % media_seqParalTime)
    print("%.5f" % desvioPadrao_seqParalTime)
    print("")

    print("%.5f" % media_paralTime)
    print("%.5f" % desvioPadrao_paralTime)
    print("")

    print("%.5f" % media_totalTime)
    print("%.5f" % desvioPadrao_totalTime)
    print("\n")

    if flagSeq == 1:
        print("Resultado: ", resultado1, ' ',resultado2)

        print("Speedup: %.5f" % (media / media_totalTime))
    print("-------------------------------------------------------------------")


    print("ParalTime: ", media_paralTime, media_totalTime)
    seqPorcent = (media_seqParalTime * 100) / media_totalTime
    seqPorcent = seqPorcent / 100
    print("porcent: %.2f" % seqPorcent)
    # print("porcent : ", seqPorcent)

    # processors = 1
    # 
    # amdal = 1 / ((1 - (seqPorcent)) / processors) + seqPorcent
    # # amdal = 1 / seqPorcent
    # print("Amdal: ", amdal)
    # print("Speed Up: ", media/media_totalTime)

    os.system("rm problems/result.out")
    os.system("rm problems/result2.out")
            


if __name__ == "__main__":
    main()
