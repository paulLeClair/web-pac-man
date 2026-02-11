import {Component, createMemo, createSignal} from 'solid-js';
import './Ghost.css'
import {Direction, EntityState, SideOfBoardPadding, TopOfBoardPadding} from "../App";

export enum GhostName {
    UNKNOWN = "unknown",
    PINKY = "pinky",
    INKY = "inky",
    BLINKY = "blinky",
    CLYDE = "clyde"
}

enum GhostOrientation {
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
    setGhostName(props.ghostName)

    const positionStyles = createMemo(() => {
        const s = props.ghostStateAccessor();
        return {
          top: `${s.y + TopOfBoardPadding}px`,
          left: `${s.x + SideOfBoardPadding}px`,
        };
    });

    const ghostOrientation = createMemo(() => {
        switch (props.ghostStateAccessor().orientation) {
            case Direction.UP: return GhostOrientation.UP;
            case Direction.DOWN: return GhostOrientation.DOWN;
            case Direction.LEFT: return GhostOrientation.LEFT;
            case Direction.RIGHT: return GhostOrientation.RIGHT;
        }
        return GhostOrientation.UP;
    })

    const ghostClassesString = createMemo(() => {
        const s = props.ghostStateAccessor()
        // TODO -> wire in animations for being scattered and being dead
        return `ghost ${ghostName()} ${ghostOrientation()}`;
    })

  return (
      <div class={ghostClassesString()} style={positionStyles()}/>
  );
};

export default Ghost;