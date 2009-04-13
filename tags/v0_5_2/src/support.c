/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * support.c
 * Copyright (C) Kevin DeKorte 2006 <kdekorte@gmail.com>
 * 
 * support.c is free software.
 * 
 * You may redistribute it and/or modify it under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option)
 * any later version.
 * 
 * support.c is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with support.c.  If not, write to:
 * 	The Free Software Foundation, Inc.,
 * 	51 Franklin Street, Fifth Floor
 * 	Boston, MA  02110-1301, USA.
 */

#include "support.h"

gint detect_playlist(gchar * filename)
{

    FILE *fp;
    gint playlist = 0;
    gchar buffer[16 * 1024];
    gint size;
	gchar **output;
	gchar *file;
	GtkTreeViewColumn *column;
	gchar *coltitle;
	
	if (g_strncasecmp(filename,"cdda://",7) == 0) {
		playlist = 1;
	} else if (g_strncasecmp(filename,"dvd://",6) == 0) {
		playlist = 1;
	} else if (g_strncasecmp(filename,"dvdnav://",9) == 0) {
		playlist = 1;
	} else {
		
		
		fp = fopen(filename, "r");
		if (path != NULL)
			g_free(path);
		path = get_path(filename);

		if (fp != NULL) {
			if (!feof(fp)) {
				memset(buffer, 0, sizeof(buffer));
				size = fread(buffer, 1, sizeof(buffer) - 1, fp);
				output = g_strsplit(buffer,"\n",0);
				if (output[0] != NULL) {
					g_strchomp(output[0]);
					g_strchug(output[0]);
				}
				// printf("buffer=%s\n",buffer);
				if (strstr(g_strdown(buffer), "[playlist]") != 0) {
					playlist = 1;
				}

				if (strstr(g_strdown(buffer), "[reference]") != 0) {
					playlist = 1;
				}

				if (strstr(g_strdown(buffer), "<asx") != 0) {
					playlist = 1;
				}

				if (strstr(g_strdown(buffer), "http://") != 0) {
					playlist = 1;
				}

				if (strstr(g_strdown(buffer), "rtsp://") != 0) {
					playlist = 1;
				}

				if (strstr(g_strdown(buffer), "pnm://") != 0) {
					playlist = 1;
				}
				if (output[0] != NULL && g_file_test(output[0], G_FILE_TEST_EXISTS)) {
					playlist = 1;
				}
				
				if (output[0] != NULL && strlen(output[0]) > 0) {
					file = g_strdup_printf("%s/%s",path,output[0]);
					if (g_file_test(file, G_FILE_TEST_EXISTS)) {
						playlist = 1;
					}
					g_free(file);
				}

				g_strfreev(output);
			}
			fclose(fp);
		}
	}
	if (verbose)
    	printf("playlist detection = %i\n", playlist);
	if (!playlist) {
		if (playlistname != NULL)
			g_free(playlistname);
		playlistname = NULL;
		if (GTK_WIDGET(list)) {
			column = gtk_tree_view_get_column(GTK_TREE_VIEW(list),0);
			coltitle = g_strdup_printf(_("Items to Play"));
			gtk_tree_view_column_set_title(column,coltitle);
			g_free(coltitle);
		}
	}		
	
    return playlist;
}

gint parse_playlist(gchar * filename)
{
	gint ret = 0;
	GtkTreeViewColumn *column;
	gchar *coltitle;
	
	// try and parse a playlist in various forms
	// if a parsing does not work then, return 0
	
	ret = parse_basic(filename);
	if (ret != 1)
		ret = parse_ram(filename);
	if (ret != 1)
		ret = parse_cdda(filename);
	if (ret != 1)
		ret = parse_dvd(filename);
	if (ret == 1) {
		if (playlistname != NULL)
			g_free(playlistname);
		if (g_strrstr(filename,"/") != NULL) {
			playlistname = g_strdup_printf("%s",g_strrstr(filename,"/") + 1);
		} else {
			playlistname = g_strdup(filename);
		}
		if (GTK_WIDGET(list)) {
			column = gtk_tree_view_get_column(GTK_TREE_VIEW(list),0);
			coltitle = g_strdup_printf(_("%s items"),playlistname);
			gtk_tree_view_column_set_title(column,coltitle);
			g_free(coltitle);
		}
	} else {
		g_free(playlistname);
		playlistname = NULL;
		if (GTK_WIDGET(list)) {
			column = gtk_tree_view_get_column(GTK_TREE_VIEW(list),0);
			coltitle = g_strdup_printf(_("Items to Play"));
			gtk_tree_view_column_set_title(column,coltitle);
			g_free(coltitle);
		}
	}		
	
	return ret;
}

// parse_basic covers .pls, .m3u and reference playlist types 
gint parse_basic(gchar * filename)
{
	FILE *fp;
    gint ret = 0;
    gchar *buffer;
	gchar **parse;
	gchar *file = NULL;
	
    fp = fopen(filename, "r");
	buffer = g_new0(gchar,1024);
	
    if (fp != NULL) {
        while (!feof(fp)) {
            memset(buffer, 0, sizeof(buffer));
            buffer = fgets(buffer, 1024, fp);
			if (buffer != NULL) {
				g_strchomp(buffer);
				g_strchug(buffer);
				//printf("buffer=%s\n",buffer);
				if (path != NULL)
					g_free(path);
				path = get_path(filename);
				//printf("path=%s\n",path);
				if (strlen(buffer) > 0)
					file = g_strdup_printf("%s/%s",path,buffer);
				
				if (g_strcasecmp(buffer, "[playlist]") == 0) {
					//printf("playlist\n");
					ret = 1;
				}else if (g_strcasecmp(buffer, "[reference]") == 0) {
					//printf("ref\n");
					ret = 1;
				}else if (g_strncasecmp(buffer, "NumberOfEntries",strlen("NumberOfEntries")) == 0) {
					//printf("num\n");
					ret = 1;
				}else if (g_strncasecmp(buffer, "Version",strlen("Version")) == 0) {
					//printf("ver\n");
					ret = 1;
				}else if (g_strncasecmp(buffer, "http://",strlen("http://")) == 0) {
					//printf("http\n");
					ret = 1;
					add_item_to_playlist(buffer,0);
				}else if (g_strncasecmp(buffer, "mms://",strlen("mms://")) == 0) {
					//printf("mms\n");
					ret = 1;
					add_item_to_playlist(buffer,0);
				} else if(g_file_test(file, G_FILE_TEST_EXISTS)) {
					//printf("ft file - %s\n", file);
					ret = 1;
					add_item_to_playlist(file,0);
				} else if(g_file_test(buffer, G_FILE_TEST_EXISTS)) {
					//printf("ft buffer - %s\n", buffer);
					ret = 1;
					add_item_to_playlist(buffer,0);
				} else if (ret == 1) {
					if (g_ascii_strncasecmp(buffer,"ref",3) == 0) { 
						parse = g_strsplit(buffer,"=",2);
						if (parse != NULL) {
							if (parse[1] != NULL) {
								g_strchomp(parse[1]);
								g_strchug(parse[1]);
								add_item_to_playlist(parse[1],0);
							}	
							g_strfreev(parse);
						}						
					} else if (g_ascii_strncasecmp(buffer,"file",4) == 0) { 
						parse = g_strsplit(buffer,"=",2);
						if (parse != NULL) {
							if (parse[1] != NULL) {
								g_strchomp(parse[1]);
								g_strchug(parse[1]);
								add_item_to_playlist(parse[1],0);
							}	
							g_strfreev(parse);
						}						
					} else {
						add_item_to_playlist(buffer,0);
					}
				}
				if (strlen(buffer) > 0)
					g_free(file);
			}
			if (ret != 1) break;
		}
	}

	g_free(buffer);
	buffer = NULL;
	return ret;
	
}

gint parse_ram(gchar* filename) {
	
    gint ac = 0;
	gint ret = 0;
	gchar **output;
	FILE *fp;
    gchar *buffer;

    fp = fopen(filename, "r");
	buffer = g_new0(gchar,1024);
	
    if (fp != NULL) {
        while (!feof(fp)) {
            memset(buffer, 0, sizeof(buffer));
            buffer = fgets(buffer, 1024, fp);
			if (buffer != NULL) {
				output = g_strsplit(buffer,"\r",0);
				ac = 0;
				while(output[ac] != NULL) {
					g_strchomp(output[ac]);
					g_strchug(output[ac]);
					if (g_strncasecmp(output[ac], "rtsp://",strlen("rtsp://")) == 0) {
							ret = 1;
							add_item_to_playlist(output[ac],0);
					}else if (g_strncasecmp(output[ac], "pnm://",strlen("pnm://")) == 0) {
							ret = 1;
							add_item_to_playlist(output[ac],0);
					}
					ac++;
				}
				g_strfreev(output);
			}
			if (ret != 1) break;
		}
	}
	return ret;
}

gint parse_cdda(gchar* filename) {
	
    GError *error;
    gint exit_status;
    gchar *stdout;
    gchar *stderr;
    gchar *av[255];
    gint ac = 0;
	gint ret = 0;
	gchar **output;
	gchar *track;
	gint num;
	
	if (g_strncasecmp(filename,"cdda://",7) != 0) {
		return 0;
	} else {
		playlist = 0;
        // run mplayer and try to get the first frame and convert it to a jpeg
        av[ac++] = g_strdup_printf("mplayer");
        av[ac++] = g_strdup_printf("-vo");
        av[ac++] = g_strdup_printf("null");
        av[ac++] = g_strdup_printf("-ao");
        av[ac++] = g_strdup_printf("null");
        av[ac++] = g_strdup_printf("-frames");
        av[ac++] = g_strdup_printf("0");
        av[ac++] = g_strdup_printf("-identify");
        av[ac++] = g_strdup_printf("cdda://");
        av[ac] = NULL;
		
		error = NULL;
		
        g_spawn_sync(NULL, av, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &stdout, &stderr,
                     &exit_status, &error);
        if (error != NULL) {
            printf("Error when running: %s\n", error->message);
            g_error_free(error);
            error = NULL;
        }
		output = g_strsplit(stdout,"\n",0);
		ac = 0;
		while(output[ac] != NULL) {
			if (g_strncasecmp(output[ac],"ID_CDDA_TRACK_",strlen("ID_CDDA_TRACK_")) == 0) {
				sscanf(output[ac], "ID_CDDA_TRACK_%i_", &num);
				track = g_strdup_printf("cdda://%i",num);
				add_item_to_playlist(track,0);
				g_free(track);
			}
			ac++;
		}
		g_strfreev(output);
		
		ret = 1;
	}
	
	return ret;
}

// This function pulls the playlist for dvd and dvdnav
gint parse_dvd(gchar* filename) {
	
    GError *error;
    gint exit_status;
    gchar *stdout;
    gchar *stderr;
    gchar *av[255];
    gint ac = 0;
	gint ret = 0;
	gchar **output;
	gchar *track;
	gint num;
	
	if (g_strncasecmp(filename,"dvd://",strlen("dvd://")) == 0 || g_strncasecmp(filename,"dvdnav://",strlen("dvdnav://")) == 0) {
		playlist = 0;
        // run mplayer and try to get the first frame and convert it to a jpeg
        av[ac++] = g_strdup_printf("mplayer");
        av[ac++] = g_strdup_printf("-vo");
        av[ac++] = g_strdup_printf("null");
        av[ac++] = g_strdup_printf("-ao");
        av[ac++] = g_strdup_printf("null");
        av[ac++] = g_strdup_printf("-frames");
        av[ac++] = g_strdup_printf("0");
        av[ac++] = g_strdup_printf("-identify");
        av[ac++] = g_strdup_printf("dvd://");
        av[ac] = NULL;
		
		error = NULL;
		
        g_spawn_sync(NULL, av, NULL, G_SPAWN_SEARCH_PATH, NULL, NULL, &stdout, &stderr,
                     &exit_status, &error);
        if (error != NULL) {
            printf("Error when running: %s\n", error->message);
            g_error_free(error);
            error = NULL;
        }
		output = g_strsplit(stdout,"\n",0);
		ac = 0;
		while(output[ac] != NULL) {
			if (g_strncasecmp(output[ac],"ID_DVD_TITLE_",strlen("ID_DVD_TITLE_")) == 0) {
				if (strstr(output[ac],"LENGTH") != NULL) {
					sscanf(output[ac], "ID_DVD_TITLE_%i", &num);
					track = g_strdup_printf("dvd://%i",num);
					add_item_to_playlist(track,0);
					g_free(track);
				}
			}
			ac++;
		}
		g_strfreev(output);
		
		ret = 1;
	} else {
		return 0;
	}
	
	return ret;
}


gboolean update_mplayer_config()
{

    GKeyFile *config = g_key_file_new();
    gchar *data;
    gchar *def;
    GError *error;
    gchar *filename;

    error = NULL;

    filename = g_strdup_printf("%s/.mplayer/config", getenv("HOME"));
    g_key_file_load_from_file(config,
                              filename,
                              G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS, &error);

    if (error != NULL) {
        //printf("%i\n%s", error->code, error->message);
        if (error->code == 4) {
            g_file_get_contents(filename, &data, NULL, NULL);
            //printf("%s\n", data);
            def = g_strconcat("[default]\n", data, NULL);
            g_file_set_contents(filename, def, -1, NULL);
            g_free(data);
            g_free(def);
            g_error_free(error);
            error = NULL;
            g_key_file_load_from_file(config,
                                      filename,
                                      G_KEY_FILE_KEEP_TRANSLATIONS,
                                      &error);
        }
    }

    if (vo != NULL && strlen(vo) != 0) {
        g_key_file_set_string(config, "gnome-mplayer", "vo", vo);
        if (g_ascii_strcasecmp(vo, "x11") == 0) {
            // make full screen work when using x11
            g_key_file_set_string(config, "gnome-mplayer", "zoom", "1");
            // make advanced video controls work on x11
            g_key_file_set_string(config, "gnome-mplayer", "vf", "eq2");
        }

        if (g_ascii_strcasecmp(vo, "xv") == 0) {
            // make advanced video controls work on xv
            g_key_file_remove_key(config, "gnome-mplayer", "zoom", NULL);
            g_key_file_set_string(config, "gnome-mplayer", "vf", "eq2");
        }

        if (g_ascii_strcasecmp(vo, "gl") == 0 || g_ascii_strcasecmp(vo, "gl2") == 0) {
            // if vf=eq2 is set and we use gl, then mplayer crashes
            g_key_file_remove_key(config, "gnome-mplayer", "zoom", NULL);
            g_key_file_remove_key(config, "gnome-mplayer", "vf", NULL);
        }


    } else {
        g_key_file_remove_key(config, "gnome-mplayer", "vo", NULL);
    }

    if (ao != NULL && strlen(ao) != 0) {
        g_key_file_set_string(config, "gnome-mplayer", "ao", ao);
    } else {
        g_key_file_remove_key(config, "gnome-mplayer", "ao", NULL);
    }

    data = g_key_file_to_data(config, NULL, NULL);
    //printf("%i\n%s", strlen(data), data);

    g_file_set_contents(filename, data, -1, NULL);
    g_free(data);
    g_free(filename);
    g_key_file_free(config);

    return TRUE;
}

gboolean read_mplayer_config()
{

    GKeyFile *config = g_key_file_new();
    gchar *data;
    gchar *def;
    GError *error;
    gchar *filename;

    error = NULL;

    filename = g_strdup_printf("%s/.mplayer/config", getenv("HOME"));
    g_key_file_load_from_file(config,
                              filename,
                              G_KEY_FILE_KEEP_TRANSLATIONS, &error);

    if (error != NULL) {
        //printf("%i\n%s\n", error->code, error->message);
        if (error->code == 4) {
            g_file_get_contents(filename, &data, NULL, NULL);
            //printf("%s\n", data);
            def = g_strconcat("[default]\n", data, NULL);
            g_file_set_contents(filename, def, -1, NULL);
            g_free(data);

            g_error_free(error);
            error = NULL;
            g_key_file_load_from_data(config,
                                      (const gchar *) def, strlen(def),
                                      G_KEY_FILE_KEEP_COMMENTS | G_KEY_FILE_KEEP_TRANSLATIONS,
                                      &error);
            g_free(def);

        }
    }

    if (vo != NULL)
        g_free(vo);
    if (ao != NULL)
        g_free(ao);
    vo = g_key_file_get_string(config, "gnome-mplayer", "vo", NULL);
    ao = g_key_file_get_string(config, "gnome-mplayer", "ao", NULL);


    g_free(filename);
    g_key_file_free(config);

	if (verbose)
    	printf("vo = %s ao = %s\n", vo, ao);
	
    return TRUE;
}

gboolean streaming_media(gchar * filename)
{
    gboolean ret;

    ret = TRUE;

    if (strstr(filename, "dvd://") != NULL) {
        ret = FALSE;
    } else if (strstr(filename, "dvdnav://") != NULL) {
        ret = FALSE;
    } else if (strstr(filename, "cdda://") != NULL) {
        ret = FALSE;
//    } else if (strstr(filename, "file://") != NULL) {
//        ret = FALSE;
    } else {
        ret = !g_file_test(filename, G_FILE_TEST_EXISTS);
    }

    return ret;
}

gboolean device_name(gchar * filename)
{
    gboolean ret;

    ret = TRUE;

    if (g_ascii_strncasecmp(filename, "dvd://",strlen("dvd://")) == 0) {
        ret = TRUE;
    } else if (g_ascii_strncasecmp(filename, "dvdnav://",strlen("dvdnav://")) == 0) {
        ret = TRUE;
    } else if (g_ascii_strncasecmp(filename, "cdda://",strlen("cdda://")) == 0) {
        ret = TRUE;
    } else {
        ret = FALSE;
    }

    return ret;
}

GtkTreeIter add_item_to_playlist(gchar *itemname,gint playlist) 
{
	gchar *desc;
	gchar *url;
	GtkTreeIter localiter;
	
	if (!device_name(itemname) && !streaming_media(itemname)) {
		if (g_strrstr(itemname,"/") != NULL) {
			desc = g_strdup_printf("%s",g_strrstr(itemname,"/")+sizeof(gchar));
		} else {
			desc = g_strdup(itemname);
		}
	} else {
		url = g_strrstr(itemname,"http://");

		if (g_ascii_strncasecmp(itemname, "cdda://",strlen("cdda://")) == 0) {
			desc = g_strdup_printf("CD Track %s",itemname + strlen("cdda://"));
		}else if (g_ascii_strncasecmp(itemname, "file://",strlen("file://")) == 0) {
			itemname = itemname + sizeof(gchar) * strlen("file://");
			desc = g_strdup_printf("%s",g_strrstr(itemname,"/")+sizeof(gchar));
		}else if (device_name(itemname)) {
			desc = g_strdup_printf("Device - %s",itemname);
		} else {
			desc = g_strdup_printf("Stream from %s",itemname);
		}
	}
	
	if (strlen(itemname) > 0) {
		gtk_list_store_append(playliststore,&localiter);
		gtk_list_store_set(playliststore,&localiter,ITEM_COLUMN,itemname,
						   DESCRIPTION_COLUMN,desc,
						   COUNT_COLUMN,0,
						   PLAYLIST_COLUMN,playlist, -1);

		gtk_list_store_append(nonrandomplayliststore,&localiter);
		gtk_list_store_set(nonrandomplayliststore,&localiter,ITEM_COLUMN,itemname,
						   DESCRIPTION_COLUMN,desc,
						   COUNT_COLUMN,0,
						   PLAYLIST_COLUMN,playlist, -1);
	}
	g_free(desc);
	return localiter;	

}

gboolean next_item_in_playlist(GtkTreeIter *iter) 
{
	
	if (!gtk_list_store_iter_is_valid(playliststore,iter)) {
		return FALSE;
	} else {
		if (gtk_tree_model_iter_next(GTK_TREE_MODEL(playliststore),iter)) {
			return TRUE;
		} else {
			return FALSE;
		}
	}
}

gboolean save_playlist_pls(gchar *filename) 
{
	FILE *contents;
	gchar *itemname;
	GtkTreeIter localiter;
	gint i = 1;
	
	contents = fopen(filename,"w");
	fprintf(contents,"[playlist]\n");
	fprintf(contents,"NumberOfEntries=%i\n",gtk_tree_model_iter_n_children(GTK_TREE_MODEL(playliststore),NULL));
	fprintf(contents,"Version=2\n");
	if (contents != NULL) {
		if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore),&localiter)) {
			do {
				gtk_tree_model_get(GTK_TREE_MODEL(playliststore), &localiter, ITEM_COLUMN,&itemname,-1);
				fprintf(contents,"File%i=%s\n",i++,itemname);
			} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(playliststore),&localiter));
		}

		fclose(contents);	
		return TRUE;
	} else {
		return FALSE;
	}
}


gboolean save_playlist_m3u(gchar *filename) 
{
	FILE *contents;
	gchar *itemname;
	GtkTreeIter localiter;
	
	contents = fopen(filename,"w");
	if (contents != NULL) {
		if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(playliststore),&localiter)) {
			do {
				gtk_tree_model_get(GTK_TREE_MODEL(playliststore), &localiter, ITEM_COLUMN,&itemname,-1);
				fprintf(contents,"%s\n",itemname);
			} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(playliststore),&localiter));
		}

		fclose(contents);	
		return TRUE;
	} else {
		return FALSE;
	}
}

gchar *get_path(gchar *filename) {

	gchar cwd[1024];
	gchar *tmp = NULL;
	gchar *path = NULL;
	
	if (g_strrstr(filename,"/") != NULL) {
		path = g_strdup(filename);
		tmp = g_strrstr(path,"/");
		tmp[0] = '\0';
	} else {
		getcwd(cwd,1024);
		path = g_strdup(cwd);
	}
	
	return path;
}	

void copy_playlist(GtkListStore *source, GtkListStore *dest) {

	GtkTreeIter sourceiter;
	GtkTreeIter destiter;
	GtkTreeIter a;
	gchar *iterfilename;
	gchar *localfilename;
	gchar *itemname;
	gchar *desc;
	gint count;
	gint playlist;
	
	gtk_tree_model_get(GTK_TREE_MODEL(dest), &iter, ITEM_COLUMN,&iterfilename,-1);
	
	gtk_list_store_clear(dest);
	if (gtk_tree_model_get_iter_first(GTK_TREE_MODEL(source),&sourceiter)) {
		do {
			gtk_tree_model_get(GTK_TREE_MODEL(source), &sourceiter, ITEM_COLUMN,&itemname,
							   DESCRIPTION_COLUMN,&desc,
							   COUNT_COLUMN,&count,
							   PLAYLIST_COLUMN,&playlist, -1);
			
			gtk_list_store_append(dest,&destiter);
			gtk_list_store_set(dest,&destiter,ITEM_COLUMN,itemname,
							   DESCRIPTION_COLUMN,desc,
							   COUNT_COLUMN,count,
							   PLAYLIST_COLUMN,playlist, -1);

		} while (gtk_tree_model_iter_next(GTK_TREE_MODEL(source),&sourceiter));
	}
	
	gtk_tree_model_get_iter_first(GTK_TREE_MODEL(dest),&a);
	do {
		gtk_tree_model_get(GTK_TREE_MODEL(dest), &a, ITEM_COLUMN,&localfilename, -1);
		// printf("iter = %s   local = %s \n",iterfilename,localfilename);
		if (g_ascii_strcasecmp(iterfilename,localfilename) == 0) {
			// we found the current iter
			break;
		}
		g_free(localfilename);
	} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(dest),&a));
	g_free(iterfilename);
	iter = a;	
}

void randomize_playlist(GtkListStore *store) {
	
	GtkTreeIter a;
	GtkTreeIter b;
	gint i;
	gint items;
	gint swapid;
	GRand *rand;
	gchar *iterfilename;
	gchar *localfilename;
	
	
	items = gtk_tree_model_iter_n_children(GTK_TREE_MODEL(store),NULL);
    rand = g_rand_new();

	if (items > 0) {
		gtk_tree_model_get(GTK_TREE_MODEL(playliststore), &iter, ITEM_COLUMN,&iterfilename,-1);
		
		for (i = 0; i < items; i++) {
			swapid = g_rand_int_range(rand, 0, items);
			if (i != swapid) {
				if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store),&a,NULL,i)) {
					if (gtk_tree_model_iter_nth_child(GTK_TREE_MODEL(store),&b,NULL,swapid)) {
						gtk_list_store_swap(store,&a,&b);
					}
				}
			}
		}

		gtk_tree_model_get_iter_first(GTK_TREE_MODEL(store),&a);
		do {
			gtk_tree_model_get(GTK_TREE_MODEL(store), &a, ITEM_COLUMN,&localfilename, -1);
			// printf("iter = %s   local = %s \n",iterfilename,localfilename);
			if (g_ascii_strcasecmp(iterfilename,localfilename) == 0) {
				// we found the current iter
				break;
			}
			g_free(localfilename);
		} while(gtk_tree_model_iter_next(GTK_TREE_MODEL(store),&a));
		g_free(iterfilename);
		iter = a;
	}
	g_rand_free(rand);
}