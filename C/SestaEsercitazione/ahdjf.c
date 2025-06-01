/* Controllo che siano passati almeno 3 parametri */
    if (argc < 4)
    {
        printf("Errore nel numero dei parametri: ho bisogno di almeno 3 parametri (nomefile1, nomefile2, ..., carattere) ma argc = %d\n", argc);
        exit(1);
    }
    
    /* Salvo il numero di file passati nella variabile N */
    N = argc - 2;

    /* Controllo che l'ultimo parametro sia un singolo carattere */
    if (strlen(argv[argc - 1]) != 1)
    {
        printf("Errore nel passaggio dei parametri: %s non e' un singolo carattere\n", argv[argc - 1]);
        exit(2);
    }
    /* Salvo nella variabile Cx l'ultimo parametro */
    Cx = argv[argc - 1][0];

    /* Alloco memoria per i due array di pipe */
    pipes_12 = (pipe_t *)malloc(N * sizeof(pipe_t));
    pipes_21 = (pipe_t *)malloc(N * sizeof(pipe_t));

    /* Controllo che l'allocazione sia andata a buon fine */
    if ((pipes_12 == NULL) || (pipes_21 == NULL))
    {
        printf("Errore nella malloc per gli array di pipe\n");
        exit(3);
    }

    /* Creo le N + N pipe */
    for (i = 0; i < N; i++)
    {
        if (pipe(pipes_12[i]) < 0)
        {
            printf("Errore nella creazione della pipe %d-esima tra primo e secondo filgio\n", i);
            exit(4);
        }

        if (pipe(pipes_21[i]) < 0)
        {
            printf("Errore nella creazione della pipe %d-esima tra secondo e primo figlio\n", i);
            exit(5);
        }
        
    }
    
    /* Creo gli N*2 processi figli */
    for (i = 0; i < 2*N; i++)
    {
        /* Controllo che la fork vada a buon fine */
        if ((pid = fork()) < 0)
        {
            printf("Errore nella creazione del processo figlio di indice i = %d\n", i);
            exit(6);
        }
        if (pid == 0)
        {
            /* Controllo se sono nei primi processi figli o nei secondi */
            if (i < N)
            {
                /* Primo processo figlio */
                /* Si sceglie di ritornare 0 in caso di errore */
                /* Chiudo tutte le pipe che non servono */
                for (j = 0; j < N; j++)
                {
                    close(pipes_12[j][0]);
                    close(pipes_21[j][1]);
                    if (j != i)
                    {
                        close(pipes_12[j][1]);
                        close(pipes_21[j][0]);
                    }
                }
                
                /* Apro in lettura il file */
                if ((fd = open(argv[i + 1], O_RDONLY)) < 0)
                {
                    printf("Errore nell'apertura del file %s\n", argv[i + 1]);
                    exit(0);
                }
                
                /* Setto cur a 0 */
                occ = 0;
                
                /* Itero un ciclo che legge tutti i caratteri dal file */
                while (read(fd, &c, 1))
                {
                    /* Controllo se il carattere letto e' quello cercato */
                    if (c == Cx)
                    {
                        /* Incremento il numero di occorrenze */
                        occ++;
                        /* Salvo la posizione del carattere per comunicarla al secondo processo figlio */
                        pos = lseek(fd, 0L, SEEK_CUR) - 1L;
                        nw = write(pipes_12[i][1], &pos, sizeof(pos));
                        if (nw != sizeof(pos))
                        {
                            printf("Errore: impossibile scrivere su pipe per il processo di indice i = %d\n", i);
                            exit(0);
                        }
                        
                        /* Leggo la posizone comunicata dal secondo processo figlio e sposto l'I/O pointer */
                        nr = read(pipes_21[i][0], &pos_letta, sizeof(pos_letta));
                        if (nr != sizeof(pos_letta))
                        {
                            break;
                        }
                        lseek(fd, pos_letta + 1L, SEEK_SET);
                    }
                }
            }
            else
            {
                /* Secondo processo figlio */
                /* Si sceglie di ritornare 0 in caso di errore */
                /* Chiudo tutte le pipe che non servono */
                for (j = 0; j < N; j++)
                {
                    close(pipes_21[j][0]);
                    close(pipes_12[j][1]);
                    if (j != 2*N - i - 1)
                    {
                        close(pipes_21[j][1]);
                        close(pipes_12[j][0]);
                    }
                }
                
                /* Apro in lettura il file associato */
                if ((fd = open(argv[2*N - i], O_RDONLY)) < 0)
                {
                    printf("Errore nell'apertura del file %s\n", argv[2*N - i]);
                    exit(0);
                }
                
                /* Setto a 0 occ*/
                occ = 0;

                /* Mi sposto alla posizione inviata dal primo processo figlio */
                nr = read(pipes_12[2*N - i - 1][0], &pos_letta, sizeof(pos_letta));
                if (nr != sizeof(pos_letta))
                {
                    printf("Errore: Impossibile leggere dalla pipe per il processo di indice %d (PRIMA LETTURA)\n", i);
                    exit(0);
                }
                lseek(fd, pos_letta + 1L, SEEK_SET);

                /* Itero un ciclo che legge tutti i caratteri dal file */
                while (read(fd, &c, 1))
                {
                    /* Controllo se il carattere letto e' quello cercato */
                    if (c == Cx)
                    {
                        /* Incremento il numero di occorrenze */
                        occ++;
                        /* Salvo la posizione letta e la comunico al primo processo */
                        pos = lseek(fd, 0L, SEEK_CUR) - 1L;
                        nw = write(pipes_21[2*N-i-1][1], &pos, sizeof(pos));
                        if (nw != sizeof(pos))
                        {
                            printf("Errore: impossibile scrivvere sulla pipe per il processo di indice i = %d\n", i);
                            exit(0);
                        }
                        
                        /* Leggo la posizione inviata dal primo processo figlio */
                        nr = read(pipes_12[2*N - i - 1][0], &pos_letta, sizeof(pos_letta));
                        if (nr != sizeof(pos_letta))
                        {
                            break;
                        }
                        lseek(fd, pos_letta + 1L, SEEK_SET);
                    }
                }
            }

            exit(occ);
        }
    }
    

    /* Codice del padre */
    /* Chiudo tutte le pipe */
    for (i = 0; i < N; i++)
    {
        close(pipes_12[i][0]);
        close(pipes_21[i][0]);
        close(pipes_12[i][1]);
        close(pipes_21[i][1]);
    }
    
    /* Il padre aspetta i figli */
    for (i = 0; i < 2*N; i++)
    {
        /* Controllo che la wait abbia successo */
        if ((pid = wait(&status)) < 0)
        {
            printf("Errore nella wait\n");
            exit(8);
        }
        if ((status & 0xFF) != 0)
        {
            printf("Il processo figlio con PID: %d e' terminato in modo anomalo\n", pid);
        }
        else
        {
            ritorno = (int)((status >> 8) & 0xFF);
            printf("Il processo figlio con PID: %d ha ritornato %d (se 0 problemi!)\n", pid, ritorno);
        }
    }
    
    exit(0);
}