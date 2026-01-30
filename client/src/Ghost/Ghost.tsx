import {Component, createSignal} from 'solid-js';
import './Ghost.css'

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

interface GhostProps {
    ghostName: GhostName
}

const Ghost: Component<GhostProps> = (props) => {
    const [ghostName, setGhostName] = createSignal(GhostName.UNKNOWN)

    const [xPixel, setPixelPositionX] = createSignal(0)
    const [yPixel, setPixelPositionY] = createSignal(0)

    const [orientation, setOrientation] = createSignal(Orientation.DOWN)

    // these are todo for the moment
    const [isScattering, setIsScattering] = createSignal(false)
    const [isDead, setIsDead] = createSignal(false)

    const positionStyles = {
        left: xPixel().toString() + "px",
        top: yPixel().toString() + "px",
    }

    setGhostName(props.ghostName)

  return (
      <div class={"ghost " + ghostName() + " " + orientation()} style={positionStyles}>

      </div>
  );
};

export default Ghost;