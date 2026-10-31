# Issue #93 — Options disponibili durante la gara

Stato: piano concordato con l'utente; punti 1–4 implementati. Punti 5–6 da completare.

## Avanzamento — punto 4

- Aggiunta la riga Vibration subito sotto Device in Controls, per il giocatore selezionato con L1/R1.
- Cross cambia la preferenza di vibrazione del giocatore selezionato tramite `NativeControls_SetPlayerVibration`, salvandola immediatamente in `controls.ini` come le altre preferenze Controls.
- Il valore ON/OFF resta visibile ma grigio e non modificabile per tastiera, dispositivo assente o gamepad senza rumble.
- La nuova API `NativeControls_CanVibrate` interroga le capacità SDL nel layer platform; nessun dettaglio SDL introdotto nel codice di gioco.
- Cambio dispositivo, hotplug e Restore Defaults mantengono la preferenza del giocatore. La sezione `[Vibration]` conserva `player1`…`player4` come booleani, predefiniti a true nei file precedenti.
- Il percorso rumble e il vecchio menu pausa usano la stessa preferenza Controls; caricamento e salvataggio memory-card non gestiscono più la vibrazione.
- Aggiornata `docs/features/CONTROLS.md` per accesso alla schermata e vibrazione.
- Aggiornate le suite Controls per preferenze per giocatore, salvataggio immediato e rilettura del file, compatibilità con file precedenti, valori non validi e indipendenza dalle opzioni memory-card.
- Dopo l'armonizzazione: 94 controlli menu Controls e 248 controlli native Controls passati; anche `test_config_menu` passato. Compilazione unity x86 e link della build di verifica riusciti.
- Tutte le 28 suite CTest passate. Compilazione unity x86 e link riusciti; build di verifica in `build/ctr_native_expanded_issue93_phase4.exe` perché l'eseguibile standard era aperto.
- L'helper `game/UI/UI_VolumeSlider.c`, inizialmente rimasto fuori dal commit della fase 3, è stato recuperato e incluso dall'utente nell'amend di quel commit (`2245b9a78`). La fase 4 è stata ripresa dallo stash dopo l'amend.
- Da verificare in gioco: posizione/valore della riga Vibration, selezione per giocatore, comportamento con un controller fisico, cambio dispositivo e persistenza al riavvio.

## Avanzamento — punto 3

- Aggiunta Audio come prima sezione personalizzata di Options, disponibile nei due contesti. L'entry `CFG_ACTION` non aggiunge chiavi al file INI.
- Nuovo `game/230/MM_AudioConfig.c`, incluso nella catena unity: volumi FX/Music/Voice e modalità mono/stereo, usando etichette localizzate senza i due punti finali e helper FullScreenMenu. Le stringhe localizzate condivise non vengono modificate.
- Riutilizzato il disegno degli slider originali, estratto in `UI_DrawVolumeSlider` (`game/UI/UI_VolumeSlider.c`) e chiamato sia da Audio sia dal vecchio menu di pausa. Conservati triangolo, cursore, colori e ordine di disegno; adattate solo posizione e larghezza al layout Audio.
- Modifica volumi con la cadenza originale (4 unità per frame tenuto, limiti 0–255 e priorità a Left); Cross/Circle cambiano mono/stereo.
- Anteprime tramite `OptionsMenu_TestSound`, interrotte passando alla modalità audio, tornando alle sezioni o riaprendo Options mentre Audio è attivo.
- Nel menu principale Music usa la musica già in riproduzione come feedback, senza avviare una seconda musica di anteprima; in pausa mantiene l'anteprima originale.
- Resa sicura l'anteprima voce nel menu principale: usa il personaggio del giocatore 1 senza leggere un pilota seguito inesistente o rimasto da una gara precedente. In gara continua a seguire la telecamera.
- All'uscita da Audio si aggiornano le opzioni in memoria tramite `RaceConfig_SaveGameOptions`. La scrittura su memory card resta nel normale percorso di salvataggio del gioco; non viene introdotto un autosalvataggio su file né una seconda rappresentazione audio in `config.ini`.
- Il vecchio menu di pausa resta operativo fino al collegamento/pulizia del punto 5; la nuova sezione è già accessibile dal menu principale.
- Test: 685 controlli nella suite Options, 73 nella suite audio HOWL. Tutte le 27 suite passate per l'implementazione iniziale; dopo il riuso degli slider e la correzione della musica dal menu principale, ricompilata la suite Options ed eseguite entrambe le suite Options/HOWL. Coperti slider originali e relativi valori, etichette senza due punti, canali separati, limiti, mono/stereo, anteprime (inclusa l'assenza di musica extra nel menu principale), uscita, snapshot/ripristino GameOptions e voce senza pilota.
- Build unity x86, link e `--version` riusciti. Il gioco standard era aperto: build di verifica in `build/ctr_native_expanded_issue93_phase3.exe`.
- Da verificare in gioco: layout e ascolto delle anteprime dal menu principale, interruzione all'uscita e persistenza attraverso un normale salvataggio/caricamento del gioco. La verifica dalla nuova pausa resta al punto 5.

## Avanzamento — punto 2

- Aggiunto `ConfigEntry.availableDuringPause`, con default read-only dalla pausa per le voci non esplicitamente abilitate.
- Abilitati controlli, HUD, opzioni di flusso concordate, grafica runtime e tutti i Developer Hacks. Inclusa VSync, aggiunta al codice dopo l'analisi iniziale e applicata a runtime da `native_platform.c`.
- Un solo predicato di editabilità governa il colore dei valori e il blocco di conferma (Cross/Circle) e slider (Left/Right).
- Formattazione dei valori mantenuta negli helper FullScreenMenu: varianti con colore esplicito per testo, ON/OFF, suffissi e percentuali; gli helper preesistenti restano wrapper con colore bianco. Config sceglie il colore e usa gli helper del rispettivo tipo.
- Valori disabilitati in grigio, mantenendo il contenuto attuale e tutte le righe/sezioni normalmente visibili; dal menu principale tutte le opzioni restano modificabili.
- Disattivare Developer Hacks dalla pausa mantiene le dipendenze originali, nasconde la sezione e riporta il cursore su una sezione valida.
- La suite `test_config_menu` ora usa la tabella e le dipendenze reali, evitando qualsiasi accesso ai file di configurazione. 599 controlli su policy, valori/colori, bool/enum/slider nei due contesti, Developer Hacks e ciclo di navigazione.
- Validazione: 27 suite CTest passate; compilazione unity x86 forzata per includere le modifiche, link e `--version` riusciti.
- L'eseguibile standard era in esecuzione: build di verifica in `build/ctr_native_expanded_issue93_phase2.exe`. L'eseguibile standard non è stato sovrascritto con questa build.
- Il nuovo menu non è ancora collegato dalla pausa (punto 5). La verifica visiva dei valori grigi in gara resta da eseguire dopo quel collegamento.

## Avanzamento — punto 1

- Branch di lavoro: `feature/93-in-race-options`.
- Aggiunta `MM_ConfigMenu_Open(returnMenu, context)` con contesti `CONFIG_MENU_MAIN` e `CONFIG_MENU_PAUSE`.
- Options conserva menu chiamante e riga selezionata, inizializza sezione/scorrimento e torna al chiamante salvando configurazione e controlli.
- Pulizia dei tap diretti e degli input RectMenu nei passaggi di schermata. Lo stato fisico dei tasti tenuti resta intatto, perché GAMEPAD lo usa per derivare i tap del frame successivo.
- Reset di picker, acquisizione tasti e scorrimento Controls all'apertura/chiusura della sessione; annullamento capture anche tornando dalle sottosezioni.
- Il menu principale usa la nuova apertura. Il collegamento dalla pausa resta al punto 5, dopo l'implementazione dell'editabilità al punto 2.
- Aggiunta suite `test_config_menu`: 31 controlli su sessioni principale/pausa, ritorno al chiamante, conservazione dei flag di pausa, input simultanei, salvataggio, riapertura e annullamento acquisizione.
- Validazione: 27 suite CTest passate. Compilazione x86 e link dell'applicazione riusciti, `--version` verificato.
- L'eseguibile standard in `build/` era in esecuzione e non sovrascrivibile: il link di verifica è stato effettuato in `build/ctr_native_expanded_issue93.exe`, lasciando aperto il gioco dell'utente. L'eseguibile standard non è stato aggiornato.
- Verifica visiva in gioco ancora da eseguire.

Issue: https://github.com/thecodingbob/ctr-native/issues/93

## Obiettivo e decisioni concordate

- Mantenere una sola voce **Options**, sia nel menu principale sia nei menu di pausa.
- Riutilizzare il menu di configurazione attuale come schermata comune.
- Spostare le impostazioni audio del menu originale in una sezione **Audio**, disponibile anche dal menu principale.
- Spostare la vibrazione nel menu **Controls**, per il giocatore selezionato.
- Dalla pausa mantenere visibili tutte le sezioni e le opzioni normalmente visibili. Le opzioni non modificabili devono mostrare il valore attuale in **grigio**.
- Le righe disabilitate restano raggiungibili con il cursore, ma conferma e frecce non ne modificano il valore.
- Includere fra le opzioni modificabili tutte quelle di flusso discusse: Skip Intros, Skip Podium Ceremonies, Skip Mask Hints, Boss Cutscenes.
- **Developer Hacks: sempre modificabili anche dalla pausa**, perché servono allo sviluppo e al testing. Questa decisione sostituisce l'esclusione proposta nel primo piano. Conservare la regola di visibilità esistente della sezione, legata a `developerHacksEnabled`; «sempre modificabili» non richiede di renderla sempre visibile. Non bloccare le singole opzioni della sezione per il loro effetto sulla gara.
- Non aggiungere una voce Config separata nel menu di pausa.

## Disponibilità durante la pausa

### Modificabili

- Audio: volume effetti, musica, voci, modalità mono/stereo.
- Controls: selezione dispositivo, associazioni, deadzone, ripristino delle associazioni e vibrazione.
- General: Show Reserves Meter, Skip Intros, Skip Podium Ceremonies.
- Adventure: Skip Mask Hints, Boss Cutscenes.
- Graphics: Increase Draw Distance, Dithering, Fullscreen, VSync, Aspect Ratio, Render Scale, Smooth Scaling, FXAA, Texture Filtering, Sub-Pixel Precision.
- Tutte le opzioni Developer Hacks quando la sezione è visibile, incluso il relativo interruttore di abilitazione.

Le opzioni di flusso agiscono al prossimo evento pertinente, senza provocare transizioni immediate.

### Visibili ma disabilitate

- Fisica del kart e comportamento delle armi, compreso Mask Mode.
- Composizione partecipanti e selezione bot.
- Impostazione delle modalità, selezione estesa Adventure, multiplayer Adventure, Chaos Mode e sblocchi.
- Save Anywhere e Auto-End Races, coerentemente con il perimetro concordato di audio/controlli/HUD/grafica/flusso.
- High Quality Multiplayer: determina gli asset caricati all'ingresso nel livello e non è applicabile immediatamente.

Usare un'abilitazione esplicita per ciascuna voce; non dedurre la disponibilità soltanto dal nome della sezione. L'eccezione Developer Hacks è intenzionale.

## Risultati dell'esplorazione

- `include/platform/native_config.h`: `NativeConfig`, `ConfigEntry`, tipi e costanti delle sezioni.
- `platform/native_config.c`: tabella `g_configEntries`, caricamento/salvataggio INI e dipendenze; disabilitare Developer Hacks ripristina le altre opzioni della sezione ai default.
- `game/230/MM_ConfigMenu.c`: menu Options attuale, sezioni contigue costruite dalla tabella, sezione Controls personalizzata. Il ritorno è attualmente fissato a `D230.menuMainMenu`; serve un riferimento al chiamante. Gli input ordinari usano il pad del giocatore 1.
- `game/230/MM_MenuFlow.c`: accesso a Options dal menu principale, attualmente assegnando direttamente `g_configMenu` a `ptrDesiredMenu`.
- `game/MAIN/MainFreeze.c`: `MainFreeze_MenuPtrDefault` gestisce la voce Options della pausa e apre attualmente `data.menuRacingWheelConfig`; `MainFreeze_MenuPtrOptions` contiene il vecchio menu audio/controller. `MainFreeze_GetMenuPtr` seleziona il menu di pausa del modo corrente.
- `game/zGlobal_Data.c`: tutti i menu di pausa hanno già Options, quindi non occorre aggiungere righe per questa funzionalità.
- `game/230/MM_ControlsConfig.c`: schermata Controls con selezione giocatore, dispositivo, rebinding, deadzone e ripristino. Gestisce input del giocatore 1 e del giocatore selezionato, picker e capture.
- `game/230/MM_FullScreenMenu.c`: disegno e navigazione condivisi; attualmente i valori ordinari sono disegnati in bianco. Estendere in modo mirato il supporto al colore dei valori disabilitati.
- `game/230/MM_ChaosConfig.c`: precedente utile di menu aperto dalla pausa con riferimento al menu di ritorno e `RECTMENU_ClearInput`.
- `game/MAIN/MainFreeze_Chaos.c`: aggiunge dinamicamente Chaos Rules nei menu Adventure; verificare convivenza con Options.
- `game/LOAD/LOAD_TenStages.c`: `highDetailSplitScreenLevel` viene determinato durante il caricamento usando `disableSplitScreenLod`; motivo del blocco di High Quality Multiplayer.
- `game/ElimBG.c`: pausa con cattura dello sfondo e backup di stato grafico; verificare cambi di aspect ratio/render scale durante questa fase.
- `game/PushBuffer.c`, `platform/native_renderer.c`, `platform/native_platform.c`: consumatori runtime delle opzioni grafiche; fullscreen viene sincronizzato con la finestra.
- `game/RaceConfig.c`: audio e vibrazione passano già dal sistema di opzioni del salvataggio. Verificare che il nuovo accesso dal menu principale mantenga il percorso corretto.
- La vibrazione è per giocatore, tramite `P1_VIBRATE`…`P4_VIBRATE` e `data.gGT_gameMode1_VibPerPlayer`. Attenzione: bit impostato significa vibrazione **disabilitata**. Il menu originale fa XOR del bit e visualizza ON/OFF con questa semantica.
- Il vecchio menu comprende anche configurazione JogCon/NegCon: investigarne i riferimenti prima di eliminare o scollegare codice condiviso. Il requisito esplicito riguarda lo spostamento della vibrazione in Controls.

## Piano operativo

### 1. Apertura e ritorno condivisi

- Aggiungere una funzione esplicita di apertura di Options con contesto (principale/pausa) e menu chiamante.
- Inizializzare selezione, sezione e scorrimento in modo coerente all'apertura.
- Dal menu principale tornare al menu principale; dalla pausa tornare al relativo menu di pausa mantenendo la sua selezione.
- Mantenere gara e audio di gara in pausa durante la navigazione; riprendere soltanto con Resume.
- Pulire gli input fra schermate e annullare capture eventualmente aperte.
- Conservare il salvataggio di configurazione e controlli all'uscita.

### 2. Disponibilità dichiarata nelle opzioni

- Aggiungere un attributo esplicito a `ConfigEntry`, ad esempio `availableDuringPause`.
- Abilitare le voci concordate, inclusa l'intera sezione Developer Hacks.
- Usare lo stesso controllo per colore del valore e blocco di tutti i percorsi di modifica: bool, enum e slider.
- Non filtrare le righe né le sezioni in base alla disponibilità dalla pausa: mantenere la struttura contigua esistente.
- Conservare le condizioni di visibilità già esistenti, in particolare Developer Hacks.
- Gestire correttamente l'uscita dalla sezione Developer Hacks quando la sua disabilitazione la nasconde e ripristina i valori.

### 3. Sezione Audio

- Integrare una sezione personalizzata Audio nel menu comune, analogamente a Controls.
- Estrarre/riutilizzare la logica audio originale: tre volumi e mono/stereo, con funzioni `howl_*` esistenti.
- Conservare il comportamento di anteprima sonora e interromperla quando si lascia la sezione.
- Usare il layout comune di Options; separare dalla logica controller del vecchio menu.
- Verificare applicazione e persistenza anche accedendo dal menu principale.

### 4. Vibrazione in Controls

- Aggiungere Vibration ON/OFF per il giocatore selezionato.
- Conservare la preferenza per giocatore in `controls.ini`, con salvataggio immediato e valore predefinito ON, uniformemente alle altre impostazioni Controls (revisione richiesta dall'utente).
- Gestire coerentemente dispositivi assenti/non compatibili con la schermata attuale.
- Verificare che cambio dispositivo e ripristino associazioni non cambino accidentalmente la preferenza di vibrazione.

### 5. Collegamenti e documentazione

- Collegare Options del menu principale e della pausa alla nuova apertura comune.
- Gestire la pulizia specifica dell'hub Adventure già presente nel percorso di pausa.
- Rimuovere/adattare soltanto il codice originale effettivamente sostituito, dopo averne verificato i riferimenti.
- Aggiornare la documentazione utente pertinente per Audio, vibrazione e disponibilità delle opzioni dalla pausa.

### 6. Validazione

- Build x86 secondo le istruzioni del repository.
- Test mirati significativi per disponibilità/editabilità e navigazione di ritorno, leggendo la skill testing prima di aggiungerli.
- Verificare che le righe grigie non cambino con alcun input e che i valori siano quelli correnti.
- Verificare che Developer Hacks restino modificabili dalla pausa, incluso spegnimento della sezione e relative dipendenze.
- Verificare audio, anteprime, vibrazione e persistenza nei rispettivi sistemi esistenti.
- Verificare apertura, navigazione, ritorno alla pausa e Resume, senza propagazione degli input.
- Verificare cambio dispositivo/rebinding, uscita dal picker e annullamento capture.
- Verificare cambi di aspect ratio, fullscreen e render scale durante la pausa, sfondo catturato e corretta ripresa.
- Coprire Arcade, Time Trial, Battle, Adventure hub/gara e coppe, split-screen e Chaos Rules.

## Stato del repository all'analisi

- Branch: `master`, allineato a `origin/master` al controllo iniziale.
- File non tracciati già presenti: `MEMORY.md`, `config.ini`, `controls.ini`, `memory/`. Preservarli.
- Nessuna modifica applicativa effettuata durante la pianificazione; per lo stato successivo vedere l'avanzamento sopra.
- Prima dell'implementazione ricontrollare lo stato Git e creare un topic branch: `master` è protetto.
- Non effettuare commit; seguire `AGENTS.md` per staging e collaborazione. Non aggiungere file locali di configurazione ai commit.
- Leggere le skill pertinenti per implementazione/validazione. Già consultate durante l'analisi: runtime-configuration, hud-and-ui, game-modes, codebase-navigation.
