import { Component, Show } from "solid-js";

interface GameHudProps {
    score: number;
    level: number;
    lives?: number;
}

const GameHud: Component<GameHudProps> = (props) => {
    return (
        <div class="cabinet-hud">
            <div class="cabinet-hud__left">SCORE {props.score}</div>
            <div class="cabinet-hud__center">LEVEL {props.level}</div>
            <Show when={props.lives !== undefined}>
                <div class="cabinet-hud__right">LIVES {props.lives}</div>
            </Show>
        </div>
    );
};

export default GameHud;