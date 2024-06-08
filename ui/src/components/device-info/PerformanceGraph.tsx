import {AreaChart, Card, Text, Title} from '@tremor/react';

export type PerformanceStats = {
    timestamp: number;
    memory: {
        total: number;
        used: number;
    };
    cpu: {
        model: string;
        cores: number;
        usage: number;
        processUsage: number;
    };
};


export type PerformanceGraphProps = {
    stats: Array<PerformanceStats>;
}

export const PerformanceGraph = ({ stats }: PerformanceGraphProps) => {
    // Prepare the data for the graph
    const memoryStats = stats.map((stat: PerformanceStats) => ({
        // Convert the timestamp to a human-readable date, it is an epoch timestamp
        name: new Date(stat.timestamp/1000000).toLocaleTimeString(),
        Total: stat.memory.total,
        Used: stat.memory.used,
    }));

    const cpuStats = stats.map((stat: PerformanceStats) => ({
        // Convert the timestamp to a human-readable date, it is an epoch timestamp
        name: new Date(stat.timestamp/1000000).toLocaleTimeString(),
        Total: stat.cpu.usage,
        Process: stat.cpu.processUsage,
    }));

    function formatMemory(value: number) {
        return `${value.toFixed(2)} GB`;
    }

    function formatCpu(value: number) {
        return `${value.toFixed(2)} %`;
    }

    return (
        <Card className={"w-svw flex"}>
            <Title className={"mx-auto"}>Performance Graph</Title>
            <Text>Memory Usage Over Time</Text>
            <AreaChart className={"w-1/3 flex"}
                valueFormatter={formatMemory}
                index="name"
                xAxisLabel="Time"
                yAxisLabel="Memory (GB)"
                yAxisWidth={50}
                data={memoryStats}
                categories={['Total', 'Used']}
            />
            <Text>CPU Usage Over Time</Text>
            <AreaChart className={"w-1/3"}
                valueFormatter={formatCpu}
                index="name"
                xAxisLabel="Time"
                yAxisLabel="CPU (usage %)"
                yAxisWidth={50}
                data={cpuStats}
                maxValue={100}
                minValue={0}
                categories={['Total', 'Process']}
            />
        </Card>
    );
};
