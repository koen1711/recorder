import React from 'react';
import { Accordion, AccordionBody, AccordionHeader } from "@tremor/react";
import Channel from "./Channel.tsx";
import {RecorderContext} from "../../providers/RecorderProvider.tsx";
import {PermissionContext} from "../../providers/PermissionProvider.tsx";

export function AudioStream() {
    let selectedChannel = -1;
    let curAudioElement: HTMLAudioElement | null = null;
    const [, , , , channelInfo] = React.useContext(RecorderContext);
    const [audioChannels, setChannels] = React.useState([]);
    const [, allowUnregisteredStreaming] = React.useContext(PermissionContext);

    async function onChannelClick(channel: {index: number}, selected: boolean) {
        if (selected && selectedChannel == -1) {
            selectedChannel = channel.index;
            //playAudioStream();
            curAudioElement = document.createElement("audio");
            curAudioElement.onerror = () => {
                selectedChannel = -1;
                curAudioElement = null;
            };
            // only return true when the audio stream starts playing
            curAudioElement.src = `/audiostream?channel=${selectedChannel}`;
            await curAudioElement.play();
            if (curAudioElement.error) {
                selectedChannel = -1;
                curAudioElement = null;
                return false;
            }
            return true;
        } else if (!selected && selectedChannel == channel.index) {
            // delete the audio element
            // @ts-ignore
            curAudioElement.pause();
            // @ts-ignore
            curAudioElement.remove();
            curAudioElement = null;
            selectedChannel = -1;
            return true;
        }
        return false;
    }


    React.useEffect(() => {
        // Remove all channelsAudio
        const channels = channelInfo.channels.map((channel, index) => (
            <Channel key={index} channel={channel} onChannelClick={onChannelClick} />
        ));
        setChannels(channels);
    }, [channelInfo.channels]);

    return (
        <Accordion hidden={!allowUnregisteredStreaming}>
            <AccordionHeader className={"text-center text-xl"}>Audio Stream</AccordionHeader>
            <AccordionBody className={"flex flex-wrap flex-row"}>
                {audioChannels}
            </AccordionBody>
        </Accordion>

    );
}
