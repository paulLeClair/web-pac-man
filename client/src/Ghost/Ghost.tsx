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

enum DeadGhostOrientation {
    UP = "dead-orientation-up",
    DOWN = "dead-orientation-down",
    LEFT = "dead-orientation-left",
    RIGHT = "dead-orientation-right"
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
          top: `${s.y + TopOfBoardPadding - 2}px`,
          left: `${s.x + SideOfBoardPadding - 2}px`,
        };
    });

    const ghostOrientation = createMemo(() => {
        switch (props.ghostStateAccessor().orientation) {
            case Direction.UP: return props.ghostStateAccessor().isDead
                ? DeadGhostOrientation.UP : GhostOrientation.UP;
            case Direction.DOWN: return props.ghostStateAccessor().isDead
                ? DeadGhostOrientation.DOWN : GhostOrientation.DOWN;
            case Direction.LEFT: return props.ghostStateAccessor().isDead
                ? DeadGhostOrientation.LEFT : GhostOrientation.LEFT;
            case Direction.RIGHT: return props.ghostStateAccessor().isDead
                ? DeadGhostOrientation.RIGHT : GhostOrientation.RIGHT;
        }
        return GhostOrientation.UP;
    })

    const isScattering = createMemo(() => {
        if (props.ghostStateAccessor().isDead) return "dead"
        if (props.isScatteringAccessor()) return "ghost-scattering-initial"
        return ""
    })

    const ghostClassesString = createMemo(() => {
        const s = props.ghostStateAccessor()
        // TODO -> wire in animations for being scattered and being dead
        return `ghost ${isScattering().length ? isScattering() : ghostName()} ${isScattering() == "ghost-scattering-initial" ? "" : ghostOrientation()}`;
    })

  return (
      <div class={ghostClassesString()} style={positionStyles()}/>
  );
};

export default Ghost;