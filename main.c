#include <gtk/gtk.h>
#include <jack/jack.h>

double crossfader=0.5, vol1=1, vol2=1;
jack_port_t *ports[6];

double crossfade(double crossfader_value, int function_number)
    {
    if (function_number==1)
        return crossfader_value;
    return crossfader_value;
    }


gboolean on_crossfader_change_value(GtkRange *range, GtkScrollType scroll, gdouble value, gpointer user_data)
{
if (value>100) value=100; if (value<0) value=0;
crossfader= value / 100;
return 0;
}

gboolean on_vol1_change_value(GtkRange *range, GtkScrollType scroll, gdouble value, gpointer user_data)
{
if (value>100) value=100; if (value<0) value=0;
vol1= value / 100;
return 0;
}

gboolean  on_vol2_change_value(GtkRange *range, GtkScrollType scroll, gdouble value, gpointer user_data)
{
if (value>100) value=100; if (value<0) value=0;
vol2= value / 100;
return 0;
}

void on_window_destroy (GtkObject *object, gpointer user_data)
    {
    gtk_main_quit ();
    }

void init_gtk(int argc, char *argv[])
    {
    GtkBuilder      *builder; 
    GtkWidget       *window;
    gtk_init (&argc, &argv);
    builder = gtk_builder_new ();
    gtk_builder_add_from_file (builder, "jackdjmixer.glade", NULL);
    window = GTK_WIDGET (gtk_builder_get_object (builder, "window"));
    gtk_builder_connect_signals (builder, NULL);
    g_object_unref (G_OBJECT (builder));
    gtk_widget_show (window);                
    gtk_main ();
    }


int process(jack_nframes_t nframes, void *notused)
	{
	int	i;
    
    for (i=0; i<6; i++)
        if (ports[i]==NULL)
		    return 0;

    jack_default_audio_sample_t *outl = (jack_default_audio_sample_t *) jack_port_get_buffer (ports[4], nframes);
    jack_default_audio_sample_t *outr = (jack_default_audio_sample_t *) jack_port_get_buffer (ports[5], nframes);
    jack_default_audio_sample_t *in1l = (jack_default_audio_sample_t *) jack_port_get_buffer (ports[0], nframes);
    jack_default_audio_sample_t *in1r = (jack_default_audio_sample_t *) jack_port_get_buffer (ports[1], nframes);
    jack_default_audio_sample_t *in2l = (jack_default_audio_sample_t *) jack_port_get_buffer (ports[2], nframes);
    jack_default_audio_sample_t *in2r = (jack_default_audio_sample_t *) jack_port_get_buffer (ports[3], nframes);
        
    for (i=0; i<nframes; i++)
        {
        *outl= (*in1l * vol1 * crossfade(1-crossfader, 1)) + (*in2l * vol2 * crossfade(crossfader, 1)) - ((*in1l * vol1 * crossfade(1-crossfader, 1))*(*in2l * vol2 * crossfade(crossfader, 1)));
        *outr= (*in1r * vol1 * crossfade(1-crossfader, 1)) + (*in2r * vol2 * crossfade(crossfader, 1)) - ((*in1r * vol1 * crossfade(1-crossfader, 1))*(*in2r * vol2 * crossfade(crossfader, 1)));
        outl++; outr++; in1l++; in1r++; in2l++; in2r++;
        }

	return 0;
	}
    

int init_jack()
	{
    jack_client_t *client;
        
	if ((client = jack_client_open("jackdjmixer", JackNullOption, NULL)) == 0)
			return 1;
	jack_set_process_callback (client, process, 0);
    ports[0] = jack_port_register (client, "left1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    ports[1] = jack_port_register (client, "right1", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    ports[2] = jack_port_register (client, "left2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    ports[3] = jack_port_register (client, "right2", JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);
    ports[4] = jack_port_register (client, "left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
    ports[5] = jack_port_register (client, "right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	if (jack_activate (client))
		return 2;
	return 0;
	}


 
int main (int argc, char *argv[])
    {
    if (init_jack()) return 1;
    init_gtk(argc, argv);
    return 0;
    }

