import {Component, createEffect, createSignal, mapArray, on} from 'solid-js';
import {createWS, createWSState} from "@solid-primitives/websocket";
import Board from "./Board/Board";
import Pacman, {PacmanState} from "./Pacman/Pacman";
import Ghost, {GhostState} from "./Ghost/Ghost";
import {GhostName} from "./Ghost/Ghost";
import {GameStateMessage} from "./protobuf/gen/game_state";
import {createKeyHold, useKeyDownEvent, KbdKey} from "@solid-primitives/keyboard";
import {UserInputMessage} from "./protobuf/gen/user_inputs";

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

export const TopOfBoardPadding = 24; // probably should get this from server but its ok for now
export const SideOfBoardPadding = 4;

enum GameMode {
    Unknown = 0,
    // Loading/Attract mode
    TitleScreen,
    Ready,

    // Core gameplay
    Playing,
    Paused,

    // Specialized states
    PacmanDying,
    LevelComplete,
    GameOver,

    // Cutscenes
    Intermission1, // Blinky chases Pac-Man
    Intermission2, // Pac-Man chases Blinky
    Intermission3, // Nicknamed ghost scene
}

export interface EntityState {
    x: number,
    y: number,
    orientation: Direction
}

enum IncomingPacketType { // incoming from client's perspective
    GameModeTransitionRequest = 0x80,
    GameStateUpdate = 0x70,
}

enum OutgoingPacketType { // outgoing from client's perspective
    UserInputPress = 0x101,
    UserInputRelease = 0x102, // maybe unused/unnecessary
    GameModeComplete = 0x103,
}

const App: Component<AppProps> = (props) => {
    // TODO -> game modes once basic mechanics are in place

    // i guess state will be top-down, where the app holds onto the actual signals which we pass along via props
    const [pacmanState, setPacmanState] = createSignal<PacmanState>({x: 0, y: 0, orientation: Direction.UP, isChomping: true})
    const [ghostsScattering, setGhostsScattering] = createSignal(false)
    const [pinkyState, setPinkyState] = createSignal<GhostState>({x: 0, y: 0, orientation: Direction.UP, isDead: false})
    const [inkyState, setInkyState] = createSignal<GhostState>({x: 0, y: 0, orientation: Direction.UP, isDead: false})
    const [blinkyState, setBlinkyState] = createSignal<GhostState>({x: 0, y: 0, orientation: Direction.UP, isDead: false})
    const [clydeState, setClydeState] = createSignal<GhostState>({x: 0, y: 0, orientation: Direction.UP, isDead: false})
    const [itemsState, setItemsState] = createSignal<Map<number, number>>(new Map())

    function handleStringMessage(event: MessageEvent<string>) {
        // this should be used to transition between cutscenes/states, which should change what the user is seeing in the frontend;
        // probably we want a "gameMode" signal which this (via the event handler callback) can change if the server instructs;
        // we also would want to send back a "finished" message and wait for the next state to be sent from the server
    }

    function handleIncomingBinaryMessage(event: MessageEvent<ArrayBuffer>) {
        // for now, we'll assume all incoming messsages are game state updates until it makes sense to add cutscenes
        if (event.data.byteLength === 0) return;

        let msgBufferView= new Uint8Array(event.data);
        // IMPORTANT: server will use first byte for packet type
        let packetType = msgBufferView[0];
        switch (packetType) {
            case IncomingPacketType.GameModeTransitionRequest:
                break;
            case IncomingPacketType.GameStateUpdate:
                handleGameStateUpdate(msgBufferView);
                break;
        }

    }

    function handleGameStateUpdate(messageBufferView: Uint8Array<ArrayBuffer>) {
        if (!(messageBufferView[0] === IncomingPacketType.GameStateUpdate)) return;

        requestAnimationFrame(() => {
            // TODO -> game mode implementation; we have a field in our state update but it's not yet used

            // after this we just follow the standardized game state data format:
            const gameState = GameStateMessage.fromBinary(messageBufferView.slice(1));

            // we should be more efficient and granular with our updates to avoid unnecessary re-renders;
            // for prototyping I'll keep it naive just to ensure that the data is being passed properly.
            // after that's working, we should diff each of these so we can avoid not setting them unnecessarily

            // TODO -> we'll need to convert from game-native coordinates to whatever the game board's
            // size actually is on the client side; that way the entirety of game logic can be serverside

            setPacmanState({
                x: gameState.pacmanPositionX,
                y: gameState.pacmanPositionY,
                orientation: gameState.pacmanOrientation,
                isChomping: gameState.pacmanIsChomping
            })

            setGhostsScattering(gameState.ghostsAreScattering)

            setPinkyState({
                x: gameState.pinkyPositionX,
                y: gameState.pinkyPositionY,
                orientation: gameState.pinkyOrientation,
                isDead: gameState.pinkyIsDead
            })

            setBlinkyState({
                x: gameState.blinkyPositionX,
                y: gameState.blinkyPositionY,
                orientation: gameState.blinkyOrientation,
                isDead: gameState.blinkyIsDead
            })

            setInkyState({
                x: gameState.inkyPositionX,
                y: gameState.inkyPositionY,
                orientation: gameState.inkyOrientation,
                isDead: gameState.inkyIsDead
            })

            setClydeState({
                x: gameState.clydePositionX,
                y: gameState.clydePositionY,
                orientation: gameState.clydeOrientation,
                isDead: gameState.clydeIsDead
            })

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

    // TODO -> take into account game mode for different layouts and animations;
    // some of these can be client-side and use the websocket for synchronization
    ws.addEventListener("message", (event) => {
        if (typeof event.data === "string") {
            handleStringMessage(event);
        }
        else {
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
                case "ArrowUp": !upKeyHeld() ? sendUserInputPacket(Direction.UP, OutgoingPacketType.UserInputPress) : 0; break;
                case "ArrowDown": !downKeyHeld() ? sendUserInputPacket(Direction.DOWN, OutgoingPacketType.UserInputPress) : 0; break;
                case "ArrowLeft": !leftKeyHeld() ? sendUserInputPacket(Direction.LEFT, OutgoingPacketType.UserInputPress) : 0; break;
                case "ArrowRight": !rightKeyHeld() ? sendUserInputPacket(Direction.RIGHT, OutgoingPacketType.UserInputPress) : 0; break;
            }
        }

        e?.preventDefault()
    })

    return (
        <div>
            {/*  TODO -> scoreboard etc*/}
            <Board itemsAccessor={itemsState}>
                <Ghost ghostName={GhostName.PINKY} ghostStateAccessor={pinkyState} isScatteringAccessor={ghostsScattering}/>
                <Ghost ghostName={GhostName.INKY} ghostStateAccessor={inkyState} isScatteringAccessor={ghostsScattering}/>
                <Ghost ghostName={GhostName.BLINKY} ghostStateAccessor={blinkyState} isScatteringAccessor={ghostsScattering}/>
                <Ghost ghostName={GhostName.CLYDE} ghostStateAccessor={clydeState} isScatteringAccessor={ghostsScattering}/>
                <Pacman pacmanStateAccessor={pacmanState} scaleFactor={1.0} />
            </Board>
            {/* TODO -> lives and what not */}
        </div>
    );
};

export default App;
