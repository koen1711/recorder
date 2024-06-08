import React from 'react';
import {Accordion, AccordionBody, AccordionHeader, Card} from "@tremor/react";
import {RecorderContext} from "../../providers/RecorderProvider.tsx";
import { JSX } from 'react/jsx-runtime';
import Channel from "./Channel.tsx";

export type AudioStats = {
    timestamp: number;
    volumeLevel: [number]
};

export function AudioStream() {
    const [,,,, channelInfo] = React.useContext(RecorderContext);
    const [audioChannels, setChannels] = React.useState([]);
    let selectedChannel = -1;

    function onChannelClick(channel: number, selected: boolean) {
        if (selected && selectedChannel == -1) {
            selectedChannel = channel;
            return true;
        } else if (!selected && selectedChannel == channel) {
            selectedChannel = -1;
            return true;
        }
        return false;
    }


    React.useEffect(() => {
        // remove all channels
        const channels: JSX.Element[] = [];
        for (let i = 0; i < channelInfo.channels.length; i++) {
            channels.push(<Channel key={i} channel={channelInfo.channels[i]} onChannelClick={onChannelClick}/>)
        }
        // @ts-ignore
        setChannels(channels);
    }, [channelInfo]);

    return (
        <Accordion>
            <AccordionHeader className={"text-center text-xl"}>Audio Stream</AccordionHeader>
            <AccordionBody className={"flex flex-wrap flex-row"}>
                {audioChannels}
            </AccordionBody>
        </Accordion>

    )
}

