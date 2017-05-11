from AntwerpenZuid import *

def main():
    print("Simulation of traffic in Antwerp")

    # Initialization of compartiments and sources
    AntwerpSouth = Source(15.40, 42.86)
    AntwerpEast = Source(56.67, 24.32)
    AntwerpenS_O = Compartiment(1252)

    for t in range(0,10):
        print("Timestep ", t)

main()
