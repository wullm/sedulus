/*******************************************************************************
 * This file is part of Sedulus.
 * Copyright (c) 2022 Willem Elbers (whe@willemelbers.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 ******************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <hdf5.h>
#include <assert.h>
#include <math.h>
#include "../include/params.h"

/* The .ini parser library is minIni */
#include "../parser/minIni.h"

int readParams(struct params *pars, const char *fname) {
     pars->Seed = ini_getl("Random", "Seed", 1, fname);
     pars->FixedModes = ini_getl("Random", "FixedModes", 0, fname);
     pars->InvertedModes = ini_getl("Random", "InvertedModes", 0, fname);

     /* Initial conditions parameters */
     pars->GenerateICs = ini_getl("InitialConditions", "Generate", 1, fname);

     pars->PartGridSize = ini_getl("Simulation", "PartGridSize", 64, fname);
     pars->MeshGridSize = ini_getl("Simulation", "MeshGridSize", 64, fname);
     pars->NeutrinosPerDim = ini_getl("Simulation", "NeutrinosPerDim", 32, fname);
     pars->BoxLength = ini_getd("Simulation", "BoxLength", 1000.0, fname);

     pars->ScaleFactorBegin = ini_getd("Simulation", "ScaleFactorBegin", 0.03125, fname);
     pars->ScaleFactorEnd = ini_getd("Simulation", "ScaleFactorEnd", 1.0, fname);
     pars->ScaleFactorStep = ini_getd("Simulation", "ScaleFactorStep", 0.05, fname);
     pars->WithCOLA = ini_getl("Simulation", "WithCOLA", 0, fname);

     /* Read strings */
     int len = DEFAULT_STRING_LENGTH;
     pars->InitialConditionsFile = malloc(len);
     pars->TransferFunctionsFile = malloc(len);
     ini_gets("InitialConditions", "File", "", pars->InitialConditionsFile, len, fname);
     ini_gets("TransferFunctions", "File", "", pars->TransferFunctionsFile, len, fname);

     /* Snapshot parameters */
     pars->SnapshotTimesString = malloc(len);
     pars->SnapshotBaseName = malloc(len);
     ini_gets("Snapshots", "OutputTimes", "", pars->SnapshotTimesString, len, fname);
     ini_gets("Snapshots", "BaseName", "", pars->SnapshotBaseName, len, fname);

     return 0;
}


int cleanParams(struct params *pars) {
    free(pars->InitialConditionsFile);
    free(pars->TransferFunctionsFile);
    free(pars->SnapshotTimesString);
    free(pars->SnapshotBaseName);

    return 0;
}


int parseArrayString(char *string, double **array, int *length) {
    /* Check that there is anything there */
    if (strlen(string) <= 0)
        return 0;

    /* Permissible delimiters */
    char delimiters[] = " ,\t\n";

    /* Make a copy of the string before it gets modified */
    char copy[DEFAULT_STRING_LENGTH];
    sprintf(copy, "%s", string);

    /* Count the number of values*/
    char *token = strtok(string, delimiters);
    int count = 0;
    while (token != NULL) {
        count++;
        token = strtok(NULL, delimiters);
    }

    *length = count;

    if (count == 0)
        return 0;

    /* Allocate memory and return the length */
    *array = calloc(count, sizeof(double));

    /* Parse the original string again */
    token = strtok(copy, delimiters);
    for (int i = 0; i < count; i++) {
        sscanf(token, "%lf", &(*array)[i]);
        token = strtok(NULL, delimiters);
    }

    return 0;
}
