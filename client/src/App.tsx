import {Component, createEffect, createSignal, on} from 'solid-js';
import {createWS, createWSState} from "@solid-primitives/websocket";
import Board from "./Board/Board";
import Pacman, {PacmanState} from "./Pacman/Pacman";
import Ghost, {GhostState} from "./Ghost/Ghost";
import {GhostName} from "./Ghost/Ghost";
import {GameStateMessage} from "./protobuf/gen/game_state_pb.d";

// TODO -> move this to a defines file probably

interface AppProps {
    serverIp: string,
    serverPort: number
}

export enum Direction {
    UP = 0,
    DOWN = 1,
    LEFT = 3,
    RIGHT = 4
}

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

interface GameState {
    currentMode: GameMode,

    // these will be nullable, which means no change occurred from last tick
    pacmanState: PacmanState,
    ghostsScattering: boolean,
    pinkyState: GhostState,
    inkyState: GhostState,
    blinkyState: GhostState,
    clydeState: GhostState
}

enum IncomingPacketType {
    GameModeTransitionRequest = 0x80,
    GameStateUpdate = 0x70,
}

enum OutgoingPacketType {
    UserInputPress,
    UserInputRelease,
    GameModeComplete
}

const App: Component<AppProps> = (props) => {
    // i guess state will be top-down, where the app holds onto the actual signals which we pass along via props
    const [pacmanState, setPacmanState] = createSignal<PacmanState>({x: 0, y: 0, orientation: Direction.UP, isChomping: true})
    const [ghostsScattering, setGhostsScattering] = createSignal(false)
    const [pinkyState, setPinkyState] = createSignal<GhostState>({x: 0, y: 0, orientation: Direction.UP, isDead: false})
    const [inkyState, setInkyState] = createSignal<GhostState>({x: 0, y: 0, orientation: Direction.UP, isDead: false})
    const [blinkyState, setBlinkyState] = createSignal<GhostState>({x: 0, y: 0, orientation: Direction.UP, isDead: false})
    const [clydeState, setClydeState] = createSignal<GhostState>({x: 0, y: 0, orientation: Direction.UP, isDead: false})

    function handleStringMessage(event: MessageEvent<string>) {
        // this should be used to transition between cutscenes/states, which should change what the user is seeing in the frontend;
        // probably we want a "gameMode" signal which this (via the event handler callback) can change if the server instructs;
        // we also would want to send back a "finished" message and wait for the next state to be sent from the server
    }

    function handleBinaryMessage(event: MessageEvent<ArrayBuffer>) {
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

        // TODO -> game mode implementation; we have a field in our state update but it's not yet used

        // after this we just follow the standardized game state data format:
        const gameState = GameStateMessage.deserializeBinary(messageBufferView.slice(1));

        // we should be more efficient and granular with our updates to avoid unnecessary re-renders;
        // for prototyping I'll keep it naive just to ensure that the data is being passed properly.
        // after that's working, we should diff each of these so we can avoid not setting them unnecessarily
        setPacmanState({
            x: gameState.getPacmanpositionx(),
            y: gameState.getPacmanpositiony(),
            orientation: gameState.getPacmanorientation(),
            isChomping: gameState.getPacmanischomping()
        })

        setGhostsScattering(gameState.getGhostsarescattering())

        setPinkyState({
            x: gameState.getPinkypositionx(),
            y: gameState.getPinkypositiony(),
            orientation: gameState.getPinkyorientation(),
            isDead: gameState.getPinkyisdead()
        })

        setBlinkyState({
            x: gameState.getBlinkypositionx(),
            y: gameState.getBlinkypositiony(),
            orientation: gameState.getBlinkyorientation(),
            isDead: gameState.getBlinkyisdead()
        })

        setInkyState({
            x: gameState.getInkypositionx(),
            y: gameState.getInkypositiony(),
            orientation: gameState.getInkyorientation(),
            isDead: gameState.getInkyisdead()
        })

        setClydeState({
            x: gameState.getClydepositionx(),
            y: gameState.getClydepositiony(),
            orientation: gameState.getClydeorientation(),
            isDead: gameState.getClydeisdead()
        })


    }

    // TODO -> use props to get server ip
    const ws = createWS("ws://127.0.0.1:80")
    ws.binaryType = "arraybuffer"
    const state = createWSState(ws)
    const states = ["Connecting", "Connected", "Disconnecting", "Disconnected"];

    ws.send("Hello from client!") // TODO -> break off a little handshake procedure for init;

    // TODO -> take into account game mode for different layouts and animations;
    // some of these can be client-side and use the websocket for synchronization
    ws.addEventListener("message", (event) => {
        if (typeof event.data === "string") {
            handleStringMessage(event);
        }
        else {
            handleBinaryMessage(event);
        }
    })

    return (
        <div>
            {/*  TODO -> scoreboard etc*/}
            <Board>
                <Pacman pacmanStateAccessor={pacmanState} />
                <Ghost ghostName={GhostName.PINKY} ghostStateAccessor={pinkyState} isScatteringAccessor={ghostsScattering}/>
                <Ghost ghostName={GhostName.INKY} ghostStateAccessor={inkyState} isScatteringAccessor={ghostsScattering}/>
                <Ghost ghostName={GhostName.BLINKY} ghostStateAccessor={blinkyState} isScatteringAccessor={ghostsScattering}/>
                <Ghost ghostName={GhostName.CLYDE} ghostStateAccessor={clydeState} isScatteringAccessor={ghostsScattering}/>
            </Board>
            {/* TODO -> lives and what not */}
        </div>
    );
};



export default App;
