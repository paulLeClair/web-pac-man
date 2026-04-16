import { Component, Show } from "solid-js";

interface GameOverlayProps {
    showReadyMessage: boolean;
    showAttractMessage: boolean;
    showGameOverMessage: boolean;
}

const GameOverlay: Component<GameOverlayProps> = (props) => {
    return (
        <div class="cabinet-message-layer">
            <Show when={props.showReadyMessage}>
                <div class="cabinet-message cabinet-message--ready">READY!</div>
            </Show>

            <Show when={!props.showReadyMessage && props.showAttractMessage}>
                <div class="cabinet-message cabinet-message--attract">INSERT COIN</div>
            </Show>

            <Show when={!props.showReadyMessage && !props.showAttractMessage && props.showGameOverMessage}>
                <div class="cabinet-message cabinet-message--gameover">GAME OVER</div>
            </Show>
        </div>
    );
};

export default GameOverlay;