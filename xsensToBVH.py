
import os
import math
import numpy as np

hierachy_txt = """HIERARCHY
ROOT pelvis
{
    OFFSET 0.0 0.0 0.0
    CHANNELS 6 Xposition Yposition Zposition Zrotation Yrotation Xrotation
    JOINT shoulderL
    {
        OFFSET -0.1 0.3 0.0
        CHANNELS 3 Zrotation Yrotation Xrotation
        JOINT elbowL
        {
            OFFSET -0.2 0.0 0.0
            CHANNELS 3 Zrotation Yrotation Xrotation
            End handL
            {
                OFFSET -0.15 0.0 0.0
            }
        }
    }
    JOINT shoulderR
    {
        OFFSET 0.1 0.3 0.0
        CHANNELS 3 Zrotation Yrotation Xrotation
        JOINT elbowR
        {
            OFFSET 0.2 0.0 0.0
            CHANNELS 3 Zrotation Yrotation Xrotation
            End handR
            {
                OFFSET 0.15 0.0 0.0
            }
        }
    }
}

MOTION
Frames: """

def multInvQuaternion(q1, q2):
    invQ2 = [q2[0], -q2[1], -q2[2], -q2[3]]

    w = invQ2[0] * q1[0] - invQ2[1] * q1[1] - invQ2[2] * q1[2] - invQ2[3] * q1[3]
    x = invQ2[0] * q1[1] + invQ2[1] * q1[0] + invQ2[2] * q1[3] - invQ2[3] * q1[2]
    y = invQ2[0] * q1[2] - invQ2[1] * q1[3] + invQ2[2] * q1[0] + invQ2[3] * q1[1]
    z = invQ2[0] * q1[3] + invQ2[1] * q1[2] - invQ2[2] * q1[1] + invQ2[3] * q1[0]

    return [w, x, y, z]

def quaternionToEuler(w, x, y, z):
    t0 = 2 * (w * x + y * z)
    t1 = 1 - 2 * (x**2 + y**2)
    theta = math.atan2(t0, t1)

    t0 = 2 * (w * y - x * z)
    phi = math.asin(t0)

    t0 = 2 * (w * z + x * y)
    t1 = 1 - 2 * (y**2 + z**2)
    psi = math.atan2(t0, t1)

    return psi, phi, theta

rotationOrder = {
    "PELV": {0: (1, 1), 1: (0, -1), 2: (2, 1)},
    "UARML": {0: (0, -1), 1: (2, 1), 2: (1, 1)},
    "FARML": {0: (0, -1), 1: (2, 1), 2: (1, 1)},
    "UARMR": {0: (0, 1), 1: (2, -1), 2: (1, 1)},
    "FARMR": {0: (0, 1), 1: (2, -1), 2: (1, 1)},
}

def main():
    print(hierachy_txt)
    labels = {}
    with open("xsensData/labels.txt", "r") as fch:
        for line in fch:
            token = line.strip().split(" ")
            labels[token[0]] = token[1]
    
    fileList = [file for file in os.listdir("xsensData") if file.startswith('MT')]

    jointObj = {}
    for file in fileList:
        fch = open("xsensData/" + file, "r")

        name = file[41:49]
        print(name)

        # entete
        for i in range(13):
            fch.readline()
        
        eulerAngleList = []
        firstQuaternion = None
        for line in fch:
            tokens = line.strip().split("\t")
            quaternion = [float(tokens[8]), 0, 0, 0]
            for i in range(3):
                vBoite = float(tokens[9+i])
                dir, sens = rotationOrder[labels[name]][i]
                quaternion[dir + 1] = sens * vBoite
            if firstQuaternion is None:
                firstQuaternion = quaternion
            eulerAngleList.append(multInvQuaternion(quaternion, firstQuaternion))
        jointObj[labels[name]] = eulerAngleList
            

        fch.close()
    
    with open("output.bvh", "w") as fch:
        fch.write(hierachy_txt)

        fch.write(str(len(jointObj["PELV"])))

        fch.write("\nFrame Time: " + str(1/60) + "\n")

        order = [("PELV", None),
                 ("UARML", "PELV"),
                 ("FARML", "UARML"),
                 ("UARMR", "PELV"),
                 ("FARMR", "UARMR")]

        for i in range(len(jointObj["PELV"])):
            fch.write("0.0 0.0 0.0 ")
            for elt, parent in order:
                string = ""
                value = []
                if parent == None:
                    value = quaternionToEuler(*jointObj[elt][i])
                elif parent in jointObj.keys():
                    quaternionParent = jointObj[parent][i]
                    quaternionElt = jointObj[elt][i]
                    value = quaternionToEuler(*multInvQuaternion(quaternionElt, quaternionParent))
                string += str(value[0] * 180 / np.pi) + " "
                string += str(value[1] * 180 / np.pi) + " "
                string += str(value[2] * 180 / np.pi) + " "
                fch.write(string)
            fch.write("\n")




if __name__ == "__main__":
    main()
