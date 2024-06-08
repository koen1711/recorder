import React from 'react';
import {WebsocketContext} from "../../providers/WebsocketProvider.tsx";
import {Accordion, AccordionBody, AccordionHeader, BarChart, Card, LineChart, Title} from "@tremor/react";
import VolumeBar from "./VolumeBar.tsx";

export type AudioStats = {
    timestamp: number;
    volumeLevel: [number]
};

export function AudioInfo() {
    const [, val] = React.useContext(WebsocketContext);
    const [audioInfoList, setAudioInfoList] = React.useState([]);

    React.useEffect(() => {
        if (val) {
            const json = JSON.parse(val);
            if (json.type === 'audio-info') {
                // if all the channels are 0, we can treat it as silence and throw the data away
                if (json.volumeLevel.every((value: number) => value === 0)) {
                    return;
                }

                // @ts-ignore
                setAudioInfoList([...audioInfoList, json]);
                if (audioInfoList.length > 100) {
                    // @ts-ignore
                    setAudioInfoList(audioInfoList.slice(1));
                }
            }
        }
    }, [val]);

    const audioStats = audioInfoList.map((stat: AudioStats) => {
        const json = {
            name: new Date(stat.timestamp/1000000).toLocaleTimeString(),
            average: 0
        }
        for (let i = 0; i < stat.volumeLevel.length; i++) {
            // @ts-ignore
            json[`C${i}`] = stat.volumeLevel[i];
            json.average += stat.volumeLevel[i];
        }
        json.average /= stat.volumeLevel.length;
        return json;
    });

    const graphCategories = audioInfoList.map((stat: AudioStats) => {
        const categories: string[] = [];
        for (let i = 0; i < stat.volumeLevel.length; i++) {
            categories.push(`C${i}`);
        }
        return categories;
    })[0];

    function formatVolume(value: number) {

        return `${value.toFixed(2)}dB`;
    }

    return (
        <Accordion>
            <AccordionHeader className={"text-center text-xl"}>Audio Info</AccordionHeader>
            <AccordionBody className={"flex"}>
                <Card className={"w-1/3 mx-auto flex"}>
                    <Title>Volume/Time</Title>
                    <LineChart className={"flex"}
                               minValue={-100}
                               maxValue={100}
                               valueFormatter={formatVolume}
                               index="name"
                               xAxisLabel="Time"
                               yAxisLabel="Volume Level"
                               yAxisWidth={50}
                               data={audioStats}
                               categories={graphCategories}
                    />
                </Card>
                <Card className={"w-1/3 mx-auto flex"}>
                    <Title>Volume</Title>
                    <BarChart
                        showAnimation={true}
                        maxValue={100}
                        minValue={-100}
                        valueFormatter={formatVolume}
                        data={[audioStats[audioStats.length - 1]]}
                        categories={graphCategories} index={"name"}
                    />

                </Card>
            </AccordionBody>
        </Accordion>

    )
}

