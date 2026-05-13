import {Component, createEffect, createSignal, mapArray, on} from 'solid-js';
import {createWS, createWSState} from "@solid-primitives/websocket";
import Board from "./Board/Board";
import Pacman, {PacmanState} from "./Pacman/Pacman";
import Ghost, {GhostState} from "./Ghost/Ghost";
import {GhostName} from "./Ghost/Ghost";
import {GameStateMessage} from "./protobuf/gen/game_state";
import {createKeyHold, useKeyDownEvent, KbdKey} from "@solid-primitives/keyboard";
import {UserInputMessage} from "./protobuf/gen/user_inputs";
import {WpmPacket} from './protobuf/gen/wpm_packet'
import GameHud from "./hud/GameHud";
import GameOverlay from "./hud/GameOverlay";
import './App.css';

// TODO -> move this to a defines file probably

interface AppProps {
    serverIp: string,
    serverPort: number
}

export enum Direction {
    NONE = 0,
    UP = 1,
    DOWN = 2,
    LEFT = 3,
    RIGHT = 4
}

// this must match up with the server
export enum SoundType {
    UNKNOWN = 0,
    INTRO_THEME,
    PACMAN_EATING,
    GHOST_ALARM,
    GHOSTS_SCATTERING,
    COFFEE_BREAK,
    SPECIAL_ITEM_GET,
    EXTRA_LIFE,
    GAME_OVER,
}

enum WpmPacketType {
    UNKNOWN = 0,
    TRIGGER_SOUND,
    LOOP_SOUND,
    STOP_SOUND,
    USER_INPUT,
    TRIGGER_CUTSCENE,
}

export const TopOfBoardPadding = 24; // probably should get this from server but its ok for now
export const SideOfBoardPadding = 4;

export interface EntityState {
    x: number,
    y: number,
    orientation: Direction,
    hidden: boolean,
}

enum IncomingPacketType { // incoming from client's perspective
    GameStateUpdate = 0x70,
    WpmPacket = 0x80,
}

enum OutgoingPacketType { // outgoing from client's perspective
    UserInputPress = 0x101,
    UserInputRelease = 0x102, // maybe unused/unnecessary
}

const App: Component<AppProps> = (props) => {
    // TODO -> game modes once basic mechanics are in place

    // i guess state will be top-down, where the app holds onto the actual signals which we pass along via props
    const [pacmanState, setPacmanState] = createSignal<PacmanState>({
        x: 0,
        y: 0,
        orientation: Direction.UP,
        isChomping: true,
        hidden: false,
        isDead: false
    })
    const [ghostsScattering, setGhostsScattering] = createSignal(false)
    const [pinkyState, setPinkyState] = createSignal<GhostState>({
        x: 0,
        y: 0,
        orientation: Direction.UP,
        isDead: false,
        hidden: false
    })
    const [inkyState, setInkyState] = createSignal<GhostState>({
        x: 0,
        y: 0,
        orientation: Direction.UP,
        isDead: false,
        hidden: false
    })
    const [blinkyState, setBlinkyState] = createSignal<GhostState>({
        x: 0,
        y: 0,
        orientation: Direction.UP,
        isDead: false,
        hidden: false
    })
    const [clydeState, setClydeState] = createSignal<GhostState>({
        x: 0,
        y: 0,
        orientation: Direction.UP,
        isDead: false,
        hidden: false
    })
    const [itemsState, setItemsState] = createSignal<Map<number, number>>(new Map())
    const [cutsceneState, setCutsceneState] = createSignal<number>(0)
    const [hideBoard, setHideBoard] = createSignal(false)
    const [showReadyMessage, setShowReadyMessage] = createSignal(false)
    const [showAttractMessage, setShowAttractMessage] = createSignal(false)
    const [showGameOverMessage, setShowGameOverMessage] = createSignal(false)
    const [score, setScore] = createSignal(0)
    const [level, setLevel] = createSignal(0)

    const currentlyPlayingSounds = new Map<HTMLAudioElement, SoundType>

    // not sure if this is needed and so it might get deleted
    function handleStringMessage(event: MessageEvent<string>) {
    }

    function handleIncomingBinaryMessage(event: MessageEvent<ArrayBuffer>) {
        // for now, we'll assume all incoming messsages are game state updates until it makes sense to add cutscenes
        if (event.data.byteLength === 0) return;

        let msgBufferView = new Uint8Array(event.data);
        // IMPORTANT: server will use first byte for packet type
        let packetType = msgBufferView[0];
        switch (packetType) {
            case IncomingPacketType.WpmPacket:
                handleWpmPacket(msgBufferView);
                break;
            case IncomingPacketType.GameStateUpdate:
                handleGameStateUpdate(msgBufferView);
                break;
        }
    }

    function handleWpmPacket(messageBufferView: Uint8Array<ArrayBuffer>) {
        if (!(messageBufferView[0] === IncomingPacketType.WpmPacket)) return;
        const packet = WpmPacket.fromBinary(messageBufferView.slice(1));

        if (packet.packetType == WpmPacketType.UNKNOWN) {
            console.error("Unknown WPM packet received; no action taken")
            return
        }
        if (packet.packetType == WpmPacketType.TRIGGER_CUTSCENE) {
            handleCutsceneTriggerPacket(packet)
            return
        }
        handleSoundControlPacket(packet)
    }

    function handleCutsceneTriggerPacket(cutsceneTriggerPacket: WpmPacket) {
        // this one has to be done carefully; we want to basically just clear the game board area and
        // play the cutscene; we do this upon receiving the trigger and until the cutscene is done playing all
        // game state updates are effectively ignored.
        setCutsceneState(cutsceneTriggerPacket.payload)

        // at this point we would want to trigger the particular cutscene based off our cutscene state;
        // upon finishing we unset our cutscene state and begin digesting game state updates again

        // placeholder: COFFEE BREAK! text?
    }

    function handleSoundControlPacket(soundPacket: WpmPacket) {
        const soundType = soundPacket.payload;
        switch (soundType) {
            case SoundType.INTRO_THEME:
                executeSoundCommand(soundType, "/assets/sound/intro_theme.wav", soundPacket.packetType);
                break
            case SoundType.PACMAN_EATING:
                executeSoundCommand(soundType, "/assets/sound/chomp.wav", soundPacket.packetType);
                break
            case SoundType.GHOST_ALARM:
                executeSoundCommand(soundType, "/assets/sound/ghost_alarm.wav", soundPacket.packetType);
                break
            case SoundType.GHOSTS_SCATTERING:
                executeSoundCommand(soundType, "/assets/sound/ghost_scatter.wav", soundPacket.packetType);
                break
            case SoundType.COFFEE_BREAK:
                executeSoundCommand(soundType, "/assets/sound/coffee_break.mp3", soundPacket.packetType);
                break
            case SoundType.SPECIAL_ITEM_GET:
                executeSoundCommand(soundType, "/assets/sound/special_item_get.wav", soundPacket.packetType);
                break
            case SoundType.EXTRA_LIFE:
                executeSoundCommand(soundType, "/assets/sound/extra_life.wav", soundPacket.packetType);
                break
            case SoundType.GAME_OVER:
                executeSoundCommand(soundType, "/assets/sound/game_over.wav", soundPacket.packetType);
                break
            default:
                console.error("Unknown sound packet type: " + soundPacket.packetType);
        }
    }

    function executeSoundCommand(soundType: SoundType, soundPath: string, wpmPacketType: WpmPacketType) {
        if (soundPath.length === 0 || wpmPacketType === WpmPacketType.UNKNOWN) return;

        const audio = new Audio(soundPath);
        // audio.volume = 0.2;
        audio.volume = 0;
        switch (wpmPacketType) {
            case WpmPacketType.TRIGGER_SOUND:
                audio.loop = false;
                audio.play().catch(
                    (reason) => {
                        console.error("Failed to trigger sound: " + reason);
                    }
                );
                currentlyPlayingSounds.set(audio, soundType);
                audio.addEventListener("ended", () => {
                    currentlyPlayingSounds.delete(audio);
                })
                break
            case WpmPacketType.LOOP_SOUND:
                audio.loop = true;
                audio.play().catch(
                    (reason) => {
                        console.error("Failed to loop sound: " + reason);
                    }
                );
                currentlyPlayingSounds.set(audio, soundType);
                break
            case WpmPacketType.STOP_SOUND:
                // i think here we will need some kind of persistent tracking of all playing sounds
                for (const [audioElement, playingSoundType] of currentlyPlayingSounds.entries()) {
                    if (playingSoundType === soundType) {
                        audioElement.pause();
                        audioElement.currentTime = 0;
                    }
                }
                break
            // TODO -> just log that this isn't one of the expected sound packet designations
            default:
                console.error("Unknown WPM packet type: " + wpmPacketType);
        }
    }

    function handleGameStateUpdate(messageBufferView: Uint8Array<ArrayBuffer>) {
        if (!(messageBufferView[0] === IncomingPacketType.GameStateUpdate)) return;

        if (cutsceneState() != 0) return;

        requestAnimationFrame(() => {
            const gameState = GameStateMessage.fromBinary(messageBufferView.slice(1));

            setPacmanState({
                x: gameState.pacmanPositionX,
                y: gameState.pacmanPositionY,
                orientation: gameState.pacmanOrientation,
                isChomping: gameState.pacmanIsChomping,
                hidden: gameState.pacmanIsHidden,
                isDead: gameState.pacmanIsDead
            })

            setGhostsScattering(gameState.ghostsAreScattering)

            setPinkyState({
                x: gameState.pinkyPositionX,
                y: gameState.pinkyPositionY,
                orientation: gameState.pinkyOrientation,
                isDead: gameState.pinkyIsDead,
                hidden: gameState.pinkyIsHidden
            })

            setBlinkyState({
                x: gameState.blinkyPositionX,
                y: gameState.blinkyPositionY,
                orientation: gameState.blinkyOrientation,
                isDead: gameState.blinkyIsDead,
                hidden: gameState.blinkyIsHidden
            })

            setInkyState({
                x: gameState.inkyPositionX,
                y: gameState.inkyPositionY,
                orientation: gameState.inkyOrientation,
                isDead: gameState.inkyIsDead,
                hidden: gameState.inkyIsHidden
            })

            setClydeState({
                x: gameState.clydePositionX,
                y: gameState.clydePositionY,
                orientation: gameState.clydeOrientation,
                isDead: gameState.clydeIsDead,
                hidden: gameState.clydeIsHidden
            })

            setHideBoard(gameState.hideBoard)
            setScore(gameState.score)
            setLevel(gameState.level)
            setShowReadyMessage(gameState.displayReadyMessage);
            setShowAttractMessage(gameState.displayAttractMessage);
            setShowGameOverMessage(gameState.displayGameOverMessage);

            const itemsMap = new Map<number, number>();
            for (const [packedCoords, type] of Object.entries(gameState.items ?? {})) {
                itemsMap.set(Number(packedCoords), type)
            }
            setItemsState(itemsMap)
        })
    }

    // TODO -> use props to get server ip
    const ws = createWS("ws://127.0.0.1:80")
    ws.binaryType = "arraybuffer"

    ws.addEventListener("message", (event) => {
        if (typeof event.data === "string") {
            handleStringMessage(event);
        } else {
            handleIncomingBinaryMessage(event);
        }
    })

    const keyDownEvent = useKeyDownEvent();
    const upKeyHeld = createKeyHold("ArrowUp", {preventDefault: true});
    const downKeyHeld = createKeyHold("ArrowDown", {preventDefault: true});
    const leftKeyHeld = createKeyHold("ArrowLeft", {preventDefault: true});
    const rightKeyHeld = createKeyHold("ArrowRight", {preventDefault: true});

    function sendUserInputPacket(dir: Direction, type: OutgoingPacketType) {
        let outgoingMessage = UserInputMessage.create({
            incomingPacketType: type,
            direction: dir,
        });
        ws.send(UserInputMessage.toBinary(outgoingMessage));
    }

    createEffect(() => {
        const e = keyDownEvent();
        if (e) {
            switch (e.key) {
                case "ArrowUp":
                    !upKeyHeld() ? sendUserInputPacket(Direction.UP, OutgoingPacketType.UserInputPress) : 0;
                    break;
                case "ArrowDown":
                    !downKeyHeld() ? sendUserInputPacket(Direction.DOWN, OutgoingPacketType.UserInputPress) : 0;
                    break;
                case "ArrowLeft":
                    !leftKeyHeld() ? sendUserInputPacket(Direction.LEFT, OutgoingPacketType.UserInputPress) : 0;
                    break;
                case "ArrowRight":
                    !rightKeyHeld() ? sendUserInputPacket(Direction.RIGHT, OutgoingPacketType.UserInputPress) : 0;
                    break;
            }
        }

        e?.preventDefault()
    })

    return (
        <div class="cabinet-screen">
            <div class="cabinet-game">
                <GameHud
                    score={score()}
                    level={level()}
                />
                <div class="cabinet-playfield">

                    <Board itemsAccessor={itemsState} hidden={cutsceneState() != 0}>
                        <Ghost ghostName={GhostName.PINKY} ghostStateAccessor={pinkyState}
                               isScatteringAccessor={ghostsScattering}/>
                        <Ghost ghostName={GhostName.INKY} ghostStateAccessor={inkyState}
                               isScatteringAccessor={ghostsScattering}/>
                        <Ghost ghostName={GhostName.BLINKY} ghostStateAccessor={blinkyState}
                               isScatteringAccessor={ghostsScattering}/>
                        <Ghost ghostName={GhostName.CLYDE} ghostStateAccessor={clydeState}
                               isScatteringAccessor={ghostsScattering}/>
                        <Pacman pacmanStateAccessor={pacmanState} scaleFactor={1.0}/>
                    </Board>

                    <GameOverlay
                        showReadyMessage={showReadyMessage()}
                        showAttractMessage={showAttractMessage()}
                        showGameOverMessage={showGameOverMessage()}
                    />
                </div>
            </div>
        </div>
    );
};

export default App;
