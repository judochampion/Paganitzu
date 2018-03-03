/*
 * Implementation of the JNI link between the C code and the java GUI.
 *
 * IMPORTANT:
 * No modification to this file is allowed!
 *
 * */

#include "gui.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

/* JNI */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <jni.h>

int ready = 0;
JNIEnv* env = NULL;
JavaVM * jvm = NULL;
jclass clsMain = NULL;
jmethodID updateLevel = NULL;
jmethodID updateLevelTile = NULL;
jmethodID getKeyBitmask = NULL;
jmethodID updateAiStatus = NULL;

JNIEnv* create_vm(JavaVM ** jvm) {
	int ret;

	JavaVMInitArgs vm_args;
	JavaVMOption options[1];
	options[0].optionString = "-Djava.class.path=PGMGUI.jar";
	vm_args.version = JNI_VERSION_1_6;
	vm_args.nOptions = 1;
	vm_args.options = options;
	vm_args.ignoreUnrecognized = 0;

	ret = JNI_CreateJavaVM(jvm, (void**)&env, &vm_args);
	if(ret < 0)
    {
		printf("gui error: Unable to launch embedded JVM!!.\n");
        exit(-1);
    }

	return env;
}

void gui_initialize() 
{
    assert(!ready);

	/* create VM */
	env = create_vm(&jvm);
	if(env == NULL) {
		printf("gui error: JVM environment is NULL!!.\n");
        exit(-1);
		return;
	}

	/* Initialize JNI vars */
	clsMain = (*env)->NewGlobalRef(env, (*env)->FindClass(env, "be/ugent/intec/ibcn/pgm/project12/jni_interface/AztecGUI"));

	if(clsMain != NULL) {
        /* printf("GUI loaded.\n"); */
		updateLevel = (*env)->GetStaticMethodID(env, clsMain,"updateLevel","(Ljava/lang/String;IIIIIIII)V");
		updateLevelTile = (*env)->GetStaticMethodID(env, clsMain,"updateLevelTile","(IIIII)V");
		getKeyBitmask = (*env)->GetStaticMethodID(env, clsMain,"getKeyBitmask","()I");
        updateAiStatus = (*env)->GetStaticMethodID(env, clsMain,"updateAiStatus","(IIIIIIII)V");
	} else {
		printf("gui error: Unable to find main GUI class!!.\n");
        exit(-1);
		return;
	}

    assert(env != NULL);
    assert(clsMain != NULL);
    assert(updateLevel != NULL);
    assert(updateLevelTile != NULL);
    assert(getKeyBitmask != NULL);
    assert(updateAiStatus != NULL);

	ready = 1;
}

void gui_clean() {
	if(!ready) {
		printf("gui error: Please call the initialize method first!");
	} else {
		ready = 0;
		if(env != NULL) {
			(*env)->DeleteGlobalRef(env, clsMain);
		}
	}
}

int get_key_bitmask()
{
    int res;
    if (!ready) gui_initialize();
    
    res = (*env)->CallStaticIntMethod(env, clsMain, getKeyBitmask); 

    if((*env)->ExceptionOccurred(env) != NULL)
    {
        printf("Exception getting key bitmap. Program will exit.\n");
        (*jvm)->DetachCurrentThread(jvm);
        (*jvm)->DestroyJavaVM(jvm);
        ready = 0;
        exit(-1);
    }

    return res;
}

void update_level_screen(LevelGraphics* graphics)
{
    if (!ready) gui_initialize();

	if(!ready) {
		printf("gui error: Please call the initialize method first!");
	} else {
		if(updateLevel != NULL && updateLevelTile != NULL) {
            int x,y;
            jstring name_java_str;
            for (x=0; x < graphics->width; x++)
            {
                for (y=0; y < graphics->height; y++)
                {
                    TileGraphics* tg = & graphics->graphics[x][y];

                    (*env)->CallStaticVoidMethod(env, clsMain, updateLevelTile,
                           x, y,
                           tg->type,
                           tg->graphic_index,
                           tg->animation_index);
                }
            }
            name_java_str = (*env)->NewStringUTF(env, graphics->name);
            (*env)->CallStaticVoidMethod(env, clsMain, updateLevel, 
                    name_java_str, 
                    graphics->lives, /* lives.   -1 == GAME OVER    -2 == do not show */
                    graphics->height, 
                    graphics->width, 
                    graphics->needed_key_count, 
                    graphics->collected_key_count, 
                    graphics->initial_gem_count, 
                    graphics->collected_gem_count,
                    graphics->score /* score.  negative == do not show */);

            (*env)->DeleteLocalRef(env, name_java_str);

            if((*env)->ExceptionOccurred(env) != NULL)
            {
                printf("Exception updating GUI. Program will exit.\n");
                (*jvm)->DetachCurrentThread(jvm);
                (*jvm)->DestroyJavaVM(jvm);
                ready = 0;
                exit(-1);
            } /*else
                printf("GUI updated.\n");*/
        } else {
			printf("gui error: JNI method call failed!!.\n");
			return;
		}
	}
}

void update_ai_screen(int searchtime_ms, int memsize, int depth, int total_states, int states_unexpanded, int total_hashtable_lookups, int total_hashtable_collisions, int hashtable_size)
{
    if (!ready) gui_initialize();
    
    (*env)->CallStaticVoidMethod(env, clsMain, updateAiStatus,
            searchtime_ms, memsize, depth, total_states,
            states_unexpanded, total_hashtable_lookups, total_hashtable_collisions, hashtable_size);

    if((*env)->ExceptionOccurred(env) != NULL)
    {
        printf("Exception updating AI status. Program will exit.\n");
        (*jvm)->DetachCurrentThread(jvm);
        (*jvm)->DestroyJavaVM(jvm);
        ready = 0;
        exit(-1);
    }
}

