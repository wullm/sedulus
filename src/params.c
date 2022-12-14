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
     pars->DoNewtonianBackscaling = ini_getl("InitialConditions", "DoNewtonianBackscaling", 1, fname);

     pars->PartGridSize = ini_getl("Simulation", "PartGridSize", 64, fname);
     pars->MeshGridSize = ini_getl("Simulation", "MeshGridSize", 64, fname);
     pars->NeutrinosPerDim = ini_getl("Simulation", "NeutrinosPerDim", 0, fname);
     pars->ForeignBufferSize = ini_getl("Simulation", "ForeignBufferSize", 3000000, fname);
     pars->BoxLength = ini_getd("Simulation", "BoxLength", 1000.0, fname);

     pars->ScaleFactorBegin = ini_getd("Simulation", "ScaleFactorBegin", 0.03125, fname);
     pars->ScaleFactorEnd = ini_getd("Simulation", "ScaleFactorEnd", 1.0, fname);
     pars->ScaleFactorStep = ini_getd("Simulation", "ScaleFactorStep", 0.05, fname);
     pars->ScaleFactorTarget = ini_getd("Simulation", "ScaleFactorTarget", 1.0, fname);

     pars->DerivativeOrder = ini_getl("Simulation", "DerivativeOrder", 4, fname);

     /* Neutrino parameters */
     pars->NeutrinoPreIntegration = ini_getl("Neutrino", "PreIntegration", 1, fname);
     pars->NeutrinoScaleFactorEarly = ini_getd("Neutrino", "ScaleFactorEarly", 0.005, fname);
     pars->NeutrinoScaleFactorEarlyStep = ini_getd("Neutrino", "ScaleFactorEarlyStep", 0.2, fname);

     /* Read strings */
     int len = DEFAULT_STRING_LENGTH;
     pars->InitialConditionsFile = malloc(len);
     pars->TransferFunctionsFile = malloc(len);
     ini_gets("InitialConditions", "File", "", pars->InitialConditionsFile, len, fname);
     ini_gets("TransferFunctions", "File", "", pars->TransferFunctionsFile, len, fname);

     /* Output time strings */
     pars->SnapshotTimesString = malloc(len);
     pars->PowerSpectrumTimesString = malloc(len);
     pars->HaloFindingTimesString = malloc(len);
     ini_gets("Snapshots", "OutputTimes", "", pars->SnapshotTimesString, len, fname);
     ini_gets("PowerSpectra", "OutputTimes", "", pars->PowerSpectrumTimesString, len, fname);
     ini_gets("HaloFinding", "OutputTimes", "", pars->HaloFindingTimesString, len, fname);

     /* Snapshot parameters */
     pars->SnapshotBaseName = malloc(len);
     ini_gets("Snapshots", "BaseName", "snap", pars->SnapshotBaseName, len, fname);

     /* Halo finding string parameters */
     pars->CatalogueBaseName = malloc(len);
     pars->SnipBaseName = malloc(len);
     ini_gets("HaloFinding", "BaseName", "catalogue", pars->CatalogueBaseName, len, fname);
     ini_gets("HaloFinding", "SnipBaseName", "snip", pars->SnipBaseName, len, fname);

     /* Halo finding parameters */
     pars->DoSphericalOverdensities = ini_getl("HaloFinding", "DoSphericalOverdensities", 1, fname);
     pars->LinkingLength = ini_getd("HaloFinding", "LinkingLength", 0.2, fname);
     pars->MinHaloParticleNum = ini_getl("HaloFinding", "MinHaloParticleNum", 20, fname);
     pars->HaloFindCellNumber = ini_getl("HaloFinding", "CellNumber", 256, fname);
     pars->FOFBufferSize =  ini_getl("HaloFinding", "FOFBufferSize", 10000, fname);
     pars->SphericalOverdensityThreshold = ini_getd("HaloFinding", "SphericalOverdensityThreshold", 200.0, fname);
     pars->SphericalOverdensityMinLookRadius = ini_getd("HaloFinding", "SphericalOverdensityMinLookRadius", 10.0, fname);
     pars->ShrinkingSphereInitialRadius = ini_getd("HaloFinding", "ShrinkingSphereInitialRadius", 0.9, fname);
     pars->ShrinkingSphereRadiusFactorCoarse = ini_getd("HaloFinding", "ShrinkingSphereRadiusFactorCoarse", 0.75, fname);
     pars->ShrinkingSphereRadiusFactor = ini_getd("HaloFinding", "ShrinkingSphereRadiusFactor", 0.95, fname);
     pars->ShrinkingSphereMassFraction = ini_getd("HaloFinding", "ShrinkingSphereMassFraction", 0.01, fname);
     pars->ShrinkingSphereMinParticleNum = ini_getl("HaloFinding", "ShrinkingSphereMinParticleNum", 100, fname);
     pars->ExportSnipshots = ini_getl("HaloFinding", "ExportSnipshots", 1, fname);
     pars->SnipshotReduceFactor =  ini_getd("HaloFinding", "SnipshotReduceFactor", 0.01, fname);
     pars->SnipshotMinParticleNum =  ini_getl("HaloFinding", "SnipshotMinParticleNum", 5, fname);

     /* Power spectrum (for on-the-fly analysis) parameters */
     pars->PowerSpectrumBins =  ini_getl("PowerSpectra", "PowerSpectrumBins", 50, fname);
     pars->PositionDependentSplits =  ini_getl("PowerSpectra", "PositionDependentSplits", 8, fname);
     pars->PowerSpectrumTypes = malloc(len);
     ini_gets("PowerSpectra", "Types", "all", pars->PowerSpectrumTypes, len, fname);

     return 0;
}


int cleanParams(struct params *pars) {
    free(pars->InitialConditionsFile);
    free(pars->TransferFunctionsFile);
    free(pars->SnapshotTimesString);
    free(pars->PowerSpectrumTimesString);
    free(pars->HaloFindingTimesString);
    free(pars->SnapshotBaseName);
    free(pars->CatalogueBaseName);
    free(pars->SnipBaseName);
    free(pars->PowerSpectrumTypes);

    return 0;
}


int parseArrayString(char *string, double **array, int *length) {
    /* Check that there is anything there */
    if (strlen(string) <= 0) {
        *length = 0;
        *array = NULL;
        return 0;
    }

    /* Permissible delimiters */
    char delimiters[] = " ,\t\n";

    /* Make a copy of the string before it gets modified */
    char copy[DEFAULT_STRING_LENGTH];
    sprintf(copy, "%s", string);

    /* Count the number of values */
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

int parseOutputList(char *string, double **output_list, int *num_outputs,
                    double a_begin, double a_end) {

    /* Parse the output times from the input string */
    parseArrayString(string, output_list, num_outputs);

    if (*num_outputs < 1) {
        return 0;
    } else if (*num_outputs > 1) {
        /* Check that the output times are in ascending order */
        for (int i = 1; i < *num_outputs; i++) {
            if ((*output_list)[i] <= (*output_list)[i - 1]) {
                printf("Output times should be in strictly ascending order.\n");
                exit(1);
            }
        }
    }

    /* Check that the first output is after the beginning and the last before the end */
    if ((*output_list)[0] < a_begin) {
        printf("The first output should be after the start of the simulation.\n");
        exit(1);
    } else if ((*output_list)[*num_outputs - 1] > a_end) {
        printf("The last output should be before the end of the simulation.\n");
        exit(1);
    }

    return 0;
}

int parseCharArrayString(char *string, char ***array, int *length) {
    /* Check that there is anything there */
    if (strlen(string) <= 0) {
        *length = 0;
        *array = NULL;
        return 0;
    }

    /* Permissible delimiters */
    char delimiters[] = " ,\t\n";

    /* Make a copy of the string before it gets modified */
    char copy[DEFAULT_STRING_LENGTH];
    sprintf(copy, "%s", string);

    /* Count the number of values */
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
    *array = calloc(count, sizeof(char*));
    int offset = 0;

    /* Parse the original string again */
    token = strtok(copy, delimiters);
    for (int i = 0; i < count; i++) {
        char value[DEFAULT_STRING_LENGTH];
        int len;

        /* Parse the sub-string */
        sscanf(token, "%s", value);
        token = strtok(NULL, delimiters);
        len = strlen(value);

        /* Insert it back into the input string and terminate it with null */
        strcpy(string + offset, value);
        string[offset + len + 1] = '\0';

        /* Store and then increment the pointer */
        (*array)[i] = string + offset;
        offset += len + 1;
    }

    return 0;
}

int parseGridTypeList(char *string, enum grid_type **type_list, int *num_types) {
    /* Parse the grid types */
    char **types_strings = NULL;
    parseCharArrayString(string, &types_strings, num_types);

    if (*num_types < 1)
        return 0;

    /* Parse the requested grid types */
    *type_list = malloc(*num_types * sizeof(enum grid_type));
    for (int i = 0; i < *num_types; i++) {
        /* Compare with possible grid types */
        int match_found = 0;
        for (int j = 0; j < num_grid_types; j++) {
            if (strcmp(types_strings[i], grid_type_names[j]) == 0) {
                (*type_list)[i] = j;
                match_found = 1;
            }
        }

        if (!match_found) {
            printf("Error: unknown power spectrum type: %s!\n", types_strings[i]);
            exit(1);
        }
    }
    free(types_strings);

    return 0;
}