import React from 'react';
import {WebsocketContext} from "../../providers/WebsocketProvider.tsx";
import DiskSizeProgress from "./DiskSizeProgress.tsx";
import {Accordion, AccordionBody, AccordionHeader} from "@tremor/react";
import {PerformanceGraph} from "./PerformanceGraph.tsx";



export function DeviceInfo() {
    const [, val] = React.useContext(WebsocketContext);
    const [deviceInfo, setDeviceInfo] = React.useState({"storage": {"total": 0, "free": 0, "available": 0}});
    // make a list of all past device info's with a max length of 100
    const [deviceInfoList, setDeviceInfoList] = React.useState([]);

    React.useEffect(() => {
        if (val) {
            const json = JSON.parse(val);
            if (json.type === 'device-info') {
                if (deviceInfoList.length !== 0) {
                    // check if every key from the previous device also exists here recursively if it doesn't exist copy the value from the previous device
                    // eslint-disable-next-line no-inner-declarations
                    function checkKeys(prev: any, current: any) {
                        for (const key in prev) {
                            if (current[key] === undefined) {
                                current[key] = prev[key];
                            } else if (typeof prev[key] === 'object') {
                                checkKeys(prev[key], current[key]);
                            }
                        }
                    }
                    checkKeys(deviceInfoList[deviceInfoList.length - 1], json);
                }
                setDeviceInfo(json);
                // @ts-ignore
                setDeviceInfoList([...deviceInfoList, json]);
                if (deviceInfoList.length > 100) {

                    // @ts-ignore
                    setDeviceInfoList(deviceInfoList.slice(1));

                }
            }
        }
    }, [val]);

    return (
        <Accordion>
            <AccordionHeader className={"text-center text-xl"}>Device Info</AccordionHeader>
            <AccordionBody>
                <DiskSizeProgress totalSize={deviceInfo["storage"]["total"]} usedSize={deviceInfo["storage"]["total"] - deviceInfo["storage"]["available"]} />
                <PerformanceGraph stats={deviceInfoList} />
            </AccordionBody>
        </Accordion>

    )
}

