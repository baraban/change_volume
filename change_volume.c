#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <errno.h>
#include <assert.h>
#include <alsa/asoundlib.h>
#include <sys/poll.h>
#include <stdint.h>

static char card[64] = "default";

#define convert_prange1(val, min, max) \
	ceil((val) * ((max) - (min)) * 0.01 + (min))
#define check_range(val, min, max) \
    ((val < min) ? (min) : (val > max) ? (max) : (val))

#define BUFDISPLAY_SIZE 64
#define GRAPH_RANGE 15

void display_volume(int vol,int max)
{
    char buf[BUFDISPLAY_SIZE];
    char buf2[GRAPH_RANGE+1];
    int vol_per,vol_graph;
    vol_graph = (vol*GRAPH_RANGE)/max;
    vol_per = (vol*100)/max;
    
    memset(buf2,'#',vol_graph);
    memset(buf2+vol_graph,'-',GRAPH_RANGE-vol_graph);
    buf2[GRAPH_RANGE] = 0;

    printf("%d%% %s\n",vol_per,buf2); 

    snprintf(buf,BUFDISPLAY_SIZE,"echo %d | %s",vol_per,buf2);
    execlp("/usr/bin/ratpoison","ratpoison","-c",buf,NULL);


}

int main(int argc, char *argv[])
{
    int err = 0;
    long pmin,pmax;
    long cur;
    long val;
    int mute = 0;
    int vol =  0;

    if(argc<2)
    {
        printf("v +/-val => set volume; m => toggle mute\n");
        return -1;
    }
    if(!strcmp(argv[1],"v"))
    {
/*        printf("set volume\n");*/
        if(argc <3)
        {
            vol = 5;
        }
        else
        {
            vol = atoi(argv[2]);
        }
/*        printf("new value = %d\n",vol);*/

    }
    else if(!strcmp(argv[1],"m"))
    {
/*        printf("toggle mute\n");*/
        mute = 1;
    }
    else
    {
        printf("v +/-val => set volume; m => toggle mute\n");
        return -1;
    }


    

	snd_mixer_selem_channel_id_t chn;
    snd_mixer_t *handle = NULL;
	snd_mixer_elem_t *elem;
    snd_mixer_selem_id_t *sid;
    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, "Master");
    
    if ((err = snd_mixer_open(&handle, 0)) < 0) {
        error("Mixer %s open error: %s\n", card, snd_strerror(err));
        return err;
    }
    if ((err = snd_mixer_attach(handle, card)) < 0) {
        error("Mixer attach %s error: %s", card, snd_strerror(err));
        snd_mixer_close(handle);
        handle = NULL;
        return err;
    }
    if ((err = snd_mixer_selem_register(handle,NULL, NULL)) < 0) {
        error("Mixer register error: %s", snd_strerror(err));
        snd_mixer_close(handle);
        handle = NULL;
        return err;
    }
    err = snd_mixer_load(handle);
    if (err < 0) {
        error("Mixer %s load error: %s", card, snd_strerror(err));
        snd_mixer_close(handle);
        handle = NULL;
        return err;
    }

	elem = snd_mixer_find_selem(handle, sid);
	if (!elem) {
		error("Unable to find simple control '%s',%i\n", snd_mixer_selem_id_get_name(sid), snd_mixer_selem_id_get_index(sid));
		snd_mixer_close(handle);
		handle = NULL;
		return -ENOENT;
	}
/*    if (snd_mixer_selem_is_enumerated(elem))*/
/*        printf("snd_mixer_selem_is_enumerated true\n");*/
/*    else*/
/*        printf("snd_mixer_selem_is_enumerated false\n");*/
    snd_mixer_selem_get_playback_volume_range(elem,&pmin,&pmax);
    if(vol)
    {
/*        printf("pmin = %d pmax = %d\n",pmin,pmax);*/
        snd_mixer_selem_get_playback_volume(elem,0,&cur);
/*        printf("current volume = %d\n",cur);*/
        val = convert_prange1(vol,pmin,pmax);
/*        printf("val = %d\n",val);*/
        val+=cur;
        val = check_range(val,pmin,pmax);
        printf("volume = %d\n",val);
        snd_mixer_selem_set_playback_volume(elem,0,val);
        display_volume(val,pmax);
    }
    else if (mute)
    {
        int ival =0;
        snd_mixer_selem_get_playback_switch(elem,0,&ival);
/*        printf("switch = %d\n",ival);*/
        if(ival)
        {
/*            printf("set mute\n");*/
            snd_mixer_selem_set_playback_switch(elem,0,0);
            display_volume(0,pmax);
        }
        else
        {
/*            printf("set unmute\n");*/
            snd_mixer_selem_set_playback_switch(elem,0,1);
            snd_mixer_selem_get_playback_volume(elem,0,&cur);
            display_volume(cur,pmax);
        }
    }
/*    printf("Simple mixer control '%s',%i\n", snd_mixer_selem_id_get_name(sid), snd_mixer_selem_id_get_index(sid));*/
    snd_mixer_close(handle);
    return 0;
}
