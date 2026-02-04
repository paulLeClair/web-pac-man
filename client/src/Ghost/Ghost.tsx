import {Component, createSignal} from 'solid-js';
import './Ghost.css'
import {Direction, EntityState} from "../App";

export enum GhostName {
    UNKNOWN = "unknown",
    PINKY = "pinky",
    INKY = "inky",
    BLINKY = "blinky",
    CLYDE = "clyde"
}

enum Orientation {
    UP = "ghost-orientation-up",
    DOWN = "ghost-orientation-down",
    LEFT = "ghost-orientation-left",
    RIGHT = "ghost-orientation-right"
}

export interface GhostState extends EntityState {
    isDead: boolean
}

interface GhostProps {
    ghostName: GhostName,
    ghostStateAccessor: () => GhostState,
    isScatteringAccessor: () => boolean
}

const Ghost: Component<GhostProps> = (props) => {
    const [ghostName, setGhostName] = createSignal(GhostName.UNKNOWN)

    const positionStyles = {
        left: props.ghostStateAccessor().x.toString() + "px",
        top: props.ghostStateAccessor().y.toString() + "px",
    }

    setGhostName(props.ghostName)

    let ghostOrientationClass = Orientation.UP;
    switch (props.ghostStateAccessor().orientation) {
        case Direction.UP: ghostOrientationClass = Orientation.UP; break;
        case Direction.DOWN: ghostOrientationClass = Orientation.DOWN; break;
        case Direction.LEFT: ghostOrientationClass = Orientation.LEFT; break;
        case Direction.RIGHT: ghostOrientationClass = Orientation.RIGHT; break;
    }

  return (
      <div class={"ghost " + ghostName() + " " + ghostOrientationClass} style={positionStyles}/>
  );
};

export default Ghost;