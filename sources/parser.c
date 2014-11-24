//
//  paser.c
//  The parser needs to accept a string from the text input
//
//  Created by ben on 07/11/2014.
//
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../includes/tower.h"
#include "../includes/actionQueueDataStructure.h"
#include "./../includes/parser.h"
#include "../includes/Information_Window.h"

int parse(char *inputString)
{
    size_t len = 1+strlen(inputString);//gets the size of s
    if(len<(sizeof(char)*3))
    {
        actionUsageError();
    }

    int numberOfChunks;
    char **commandArray = breakUpString(inputString, &numberOfChunks, " ");//array of strings, each elem holds a token from the input command
    if(numberOfChunks<2|| numberOfChunks>3)
    {
        return 0;//no valid commands with less than 2 strings or more than 3
    }
#if ENABLE_TESTING
    testCommandArray(commandArray, numberOfChunks);
#endif
    //enumerated type commandType can describe each of the possible commands(see actionQueue.h)
    
    commandType action = getAction(commandArray[0]);//the first string in the command should contain the action
    
    if(action==commandError)//if getAction returns commandError then the input is invalid
    {                //Error messaging handled in getAction function
        return 0;
    }
    
#if ENABLE_TESTING
    testGetAction(action);
#endif
    int parseReturns=0;
    /**** Now we deal with each possible command separately as they all have different syntax ****/
    switch (action)
    {
        case upgrade :
            if(numberOfChunks<3)
            {
                actionUsageError();
                return 0;
            }
            parseReturns = parseUpgrade(commandArray, numberOfChunks);
            return parseReturns;
        case cat:
            parseReturns = parseCat(commandArray[1]);
            return parseReturns;
        case man:
            parseReturns = parseMan(commandArray[1]);
            return parseReturns;
        case execute:
        case set:
        default:

            fprintf(stderr,"\n***parsing not implemented yet returning***\n");
            return 0;
    }
    
    
    
    freeCommandArray(commandArray, numberOfChunks);
}


int parseMan(char * inputStringCommandMan)
{
    commandType commandToMan = getAction(inputStringCommandMan);
    
    //man(commandToMan);
    return 0;
    
    
}


int parseCat(char * inputStringTargeting)
{
    if(inputStringTargeting[0]=='t' || inputStringTargeting[0]=='T')
    {
        unsigned int targetTower = getTargetTower(inputStringTargeting);
        if(targetTower!=0)
        {
            catTower(targetTower);
            return 1;
        }
        else
            return 0;
    }
    else
        return 0;
    //can we also cat other things eg enemies?
    
}

int parseUpgrade(char ** commandArray, int numberOfChunks)
{
    
    upgradeStat statToUpgrade = getUpgradeStats(commandArray[1]);
#if ENABLE_TESTING
    testGetUpgradeStat(statToUpgrade);
#endif
    
    int target = getTargetTower(commandArray[2]);
    
    if(target!=0 && statToUpgrade!=statError )
    {
        commandType action = upgrade;
        pushToQueue(getQueue(NULL),action,statToUpgrade,target);
        return 1;
    }
    else
    {
        return 0;
    }

}


unsigned int getTargetTower(const char * inputStringTargeting)
{
    unsigned int numberOfTowers = getNumberOfTowers();//getNumberOfTowers(); this is func in tower.c

    size_t len = strlen(inputStringTargeting);//gets the size of string
    if( len<(2*sizeof(char)) )
    {
        fprintf(stderr,"*** ERROR: You must target a tower with this command ***\n");
        fprintf(stderr,"to target a tower enter t followed by a number 1 - %d \n",numberOfTowers);
        return 0;
    }
    if (inputStringTargeting[0]!='t' && inputStringTargeting[0]!='T')
    {
        fprintf(stderr,"*** ERROR: You must target a tower with this command ***\n");
        fprintf(stderr,"to target a tower enter t followed by a number 1 - %d \n",numberOfTowers);
        return 0;
    }
    
    unsigned int targetTower = (unsigned int)(inputStringTargeting[1]-'0');
    
    if(targetTower > numberOfTowers)
    {
        fprintf(stderr,"*** ERROR: target tower does not exist ***\n");
        fprintf(stderr,"You have only %d towers you entered t%d\n",
                numberOfTowers,targetTower);
        return 0;
    }
#if ENABLE_TESTING
    printf(">>>>>>>>>>>In getTargetTower\n");
    printf(">>>>>>>>>>>string: %s\n", inputStringTargeting);
    printf(">>>>>>>>>>>char to return: %c\n", inputStringTargeting[1]);
    printf(">>>>>>>>>>>int to return: %d\n", targetTower);
#endif
    
    return targetTower;
}


upgradeStat getUpgradeStats(const char * inputStringUpgradeStats)
{
    /*first lets make an array of strings to hold all the possible action commands*/
    const char **validUpgradeStats;
    int numberOfStats=5;//have 5 action commands at this time: upgrade, execute, set, man, cat
    validUpgradeStats=(const char **)malloc(numberOfStats*sizeof(char*));//array of $[numberOfActions] strings
    validUpgradeStats[0]="p";
    validUpgradeStats[1]="r";
    validUpgradeStats[2]="s";
    validUpgradeStats[3]="AOEr";
    validUpgradeStats[4]="AOEp";
    //now test the input string against all valid stats
    upgradeStat statToUpgrade=statError;
    for(int i=0; i<numberOfStats; ++i)
    {
        if(strcmp(inputStringUpgradeStats,validUpgradeStats[i])==0)//if the string is identical to one of the commands
        {                                        //then action is set to that command
            switch (i)
            {
                case 0:
                    statToUpgrade = power;
                    return statToUpgrade;
                case 1:
                    statToUpgrade = range;
                    return statToUpgrade;
                case 2:
                    statToUpgrade = speed;
                    return statToUpgrade;
                case 3:
                    statToUpgrade = AOErange;
                    return statToUpgrade;
                case 4:
                    statToUpgrade = AOEpower;
                    return statToUpgrade;
            }
        }
    }
    
    if(statToUpgrade==statError)//if it is still set to ERROR then the user made a mistake
    {
        upgrageStatUsageError(statToUpgrade, validUpgradeStats, numberOfStats);
    }
    free(validUpgradeStats);//free the mallocd array
    return statToUpgrade;
}




/* if there was a syntax error in the users command call this function which will print usage advice to the terminal window*/
void upgrageStatUsageError(upgradeStat statToUpgrade, const char ** validUpgradeStats, int numberOfStats)
{
    if(statToUpgrade==statError)//if it is still set to ERROR then the user made a mistake
    {
        fprintf(stderr,"*** stat not recognised ***\n");
        fprintf(stderr,"Possible stats: \n");
        for(int i=0; i<numberOfStats; ++i)
        {
            fprintf(stderr,"%s\n",validUpgradeStats[i]);
        }
        fprintf(stderr,"\nType man [COMMAND] for usage\n");//we advise them on usage
    }//error messages will need to be passed back to the terminal to be printed. hopefully can do this by setting up a custom stream. For now will print to stderr.
}






/* Takes the first string of the input command and tests it against the correct syntax to find which command they want to execute then returns that command as a enum commandType variable  */
commandType getAction( const char * inputAction )
{
    /*first lets make an array of strings to hold all the possible action commands*/
    const char **validActions;
    int numberOfActions=5;//have 5 action commands at this time: upgrade, execute, set, man, cat
    validActions=(const char **)malloc(numberOfActions*sizeof(char*));//array of $[numberOfActions] strings
    validActions[0]="upgrade";
    validActions[1]="execute";
    validActions[2]="set";
    validActions[3]="man";
    validActions[4]="cat";
    //now test the input string against all valid actions
    commandType action = commandError;
    for(int i=0; i<numberOfActions; ++i)
    {
        if(strcmp(inputAction,validActions[i])==0)//if the string is identical to one of the commands
        {                                        //then action is set to that command
            switch (i)
            {
                case 0:
                    action = upgrade;
                    return action;
                case 1:
                    action = execute;
                    return action;
                case 2:
                    action = set;
                    return action;
                case 3:
                    action = man;
                    return action;
                case 4:
                    action = cat;
                    return action;
                
            }
        }
    }
    
    if(action==commandError)//if it is still set to ERROR then the user made a mistake
    {
        actionUsageError();
    }
    free(validActions);//free the mallocd array
    return action;
}



/* if there was a syntax error in the users command call this function which will print usage advice to the terminal window*/
void actionUsageError()
{
    const char **validActions;
    int numberOfActions=5;//have 5 action commands at this time: upgrade, execute, set, man, cat
    validActions=(const char **)malloc(numberOfActions*sizeof(char*));//array of $[numberOfActions] strings
    validActions[0]="upgrade";
    validActions[1]="execute";
    validActions[2]="set";
    validActions[3]="man";
    validActions[4]="cat";
    
    fprintf(stderr,"*** Action not recognised ***\n");
    fprintf(stderr,"Possible commands: \n");
    for(int i=0; i<numberOfActions; ++i)
    {
        fprintf(stderr,"%s\n",validActions[i]);
    }
    fprintf(stderr,"\nType man [COMMAND] for usage\n");//we advise them on usage
    //error messages will need to be passed back to the terminal to be printed. hopefully can do this by setting up a custom stream. For now will print to stderr.
    free(validActions);//free the mallocd array

}



/* Takes the input string and breaks into separate words (where there is a space and new string starts) each of these words is stored in the commandArray which is an array of strings*/
char **breakUpString(const char * inputString, int *numberOfChunksPtr, const char * delimiter)
{
    char    *stringChunk,                       //holds the chunks on the input string as we break it up
            *inputStringDuplicate = strdup(inputString),//duplicate input string for editting
            **commandArray = NULL;              //this will be an array to hold each of the chunk strings
    int     numberOfChunks=0;
    
    //using http://www.tutorialspoint.com/c_standard_library/c_function_strtok.htm
    stringChunk = strtok(inputStringDuplicate, delimiter); // gets the first chunk (up to the first space)
    
    // walk through rest of string
    while( stringChunk != NULL )
    {
        ++numberOfChunks;
        commandArray=(char **)realloc(commandArray,numberOfChunks*sizeof(char*));//array of strings
        commandArray[numberOfChunks-1]=(char *)malloc((size_t)(strlen(stringChunk)*sizeof(char)+1));
        strcpy(commandArray[numberOfChunks-1],stringChunk);
        
        stringChunk = strtok(NULL, delimiter);
    }
    free(inputStringDuplicate);//frees the malloc made in strdup()
                               //$(numberOfChunks) strings now stored in the commandArray
    *numberOfChunksPtr=numberOfChunks;
    return commandArray;
}

/* duplicates a string */
char *strdup(const char * s)
{
    size_t len = 1+strlen(s);//gets the size of s
    char *p = malloc(len);//allocates a block big enough to hold s
    
    return p ? memcpy(p, s, len) : NULL;//if p is non 0 ie malloc worked, then copy everything in s into p and return p. if p is NULL malloc didnt work so return NULL
}

/*frees the memory allocated in breakup string funct*/
void freeCommandArray(char **commandArray,int numberOfChunks)
{
    for(int i=0; i<numberOfChunks; ++i)
    {
        free(commandArray[i]);
    }
    free(commandArray);
}

/* test function. Prints contents of commandArray*/
void testCommandArray(char ** commandArray, int numberOfChunks)
{
    for(int i=0; i<numberOfChunks; ++i)
    {
        printf("%s",commandArray[i]);
        printf("|\n");
    }
}

void testGetAction(commandType action)
{
    /*first lets make an array of strings to hold all the possible action commands*/
    const char **validActions;
    int numberOfActions=5;//have 5 action commands at this time: upgrade, execute, set, man, cat
    validActions=(const char **)malloc(numberOfActions*sizeof(char*));//array of $[numberOfActions] strings
    validActions[0]="upgrade";
    validActions[1]="execute";
    validActions[2]="set";
    validActions[3]="man";
    validActions[4]="cat";
    printf("****testGetAction****\n");
    printf("read action: %s\n", validActions[action]);
    free(validActions);//free the mallocd array

}
void testGetUpgradeStat(upgradeStat statToUpgrade)
{
    /*first lets make an array of strings to hold all the possible action commands*/
    const char **validUpgradeStats;
    int numberOfStats=5;//have 5 action commands at this time: upgrade, execute, set, man, cat
    validUpgradeStats=(const char **)malloc(numberOfStats*sizeof(char*));//array of $[numberOfActions] strings
    validUpgradeStats[0]="p";
    validUpgradeStats[1]="r";
    validUpgradeStats[2]="s";
    validUpgradeStats[3]="AOEr";
    validUpgradeStats[4]="AOEp";
    printf("****testGetUpgradeStat****\n");
    printf("read stat: %s\n", validUpgradeStats[statToUpgrade]);
    free(validUpgradeStats);//free the mallocd array
    
}
