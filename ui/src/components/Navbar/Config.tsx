import React, { useMemo, useCallback, memo, useState, useEffect } from 'react';
import {
    Accordion,
    AccordionBody,
    AccordionHeader,
    AccordionList,
    Card,
    NumberInput,
    Select,
    SelectItem,
    Tab,
    TabGroup,
    TabList,
    TabPanel,
    TabPanels, TextInput,
} from '@tremor/react';

const TabsSection = memo(({ section, renderConfig, stack, configMistake }) => (
    <Card>
        <TabGroup>
            <TabList variant="solid" defaultValue="1">
                {section.questions && Object.entries(section.questions).map(([key, value]) => (
                    <Tab className="text-xl" key={key}>{value.title}</Tab>
                ))}
            </TabList>
            <TabPanels>
                {section.questions && Object.entries(section.questions).map(([key, value]) => (
                    <TabPanel key={key}>{renderConfig(value, [...stack, 'questions'], key, configMistake)}</TabPanel>
                ))}
            </TabPanels>
        </TabGroup>
    </Card>
));

const SectionAccordion = memo(({ section, renderConfig, stack, configMistake }) => (
    <AccordionList>
        {section.questions &&
            // Sort the questions based on their order of appearance
            Object.entries(section.questions)
                .sort(([, questionA], [, questionB]) => questionA.title - questionB.title)
                .map(([key, value]) => (
                    <Accordion key={key}>
                        <AccordionHeader>{value.title}</AccordionHeader>
                        <AccordionBody>
                            {renderConfig(value, [...stack, 'questions'], key, configMistake)}
                        </AccordionBody>
                    </Accordion>
                ))}
    </AccordionList>
));


const BodySection = memo(({ section, renderConfig, stack, configMistake }) => (
    <>
        {section.questions && Object.entries(section.questions).map(([key, value]) => (
            <React.Fragment key={key}>
                {renderConfig(value, [...stack, 'questions'], key, configMistake)}
            </React.Fragment>
        ))}
    </>
));

const SelectInput = memo(({ section, stack, updateConfig, configMistake }) => {
    const [value, setValue] = useState(section.value);

    const handleChange = (e) => {
        setValue(e);
        updateConfig(stack, e);
    };

    useEffect(() => {
        setValue(section.value);
    }, [section.value]);

    const isMistake = stack[stack.length - 1] === configMistake;

    return (
        <>
            <label>{section.title}</label>
            <Select value={value} onValueChange={handleChange}>
                {section.options.map((option, idx) => (
                    <SelectItem key={idx} value={option}>{option}</SelectItem>
                ))}
            </Select>
            {isMistake && <p className="error-text">There is a mistake here.</p>}
        </>
    );
});

const NumberInputField = memo(({ section, stack, updateConfig, configMistake }) => {
    const [value, setValue] = useState(section.value);

    const handleChange = (e) => {
        setValue(e.target.value);
        updateConfig(stack, e.target.value);
    };

    useEffect(() => {
        setValue(section.value);
    }, [section.value]);

    const isMistake = stack[stack.length - 1] === configMistake;

    return (
        <>
            <label>{section.title}</label>
            <NumberInput error={isMistake} value={value} onChange={handleChange} />
        </>
    );
});

const StringInputField = memo(({ section, stack, updateConfig, configMistake }) => {
    const [value, setValue] = useState(section.value);

    const handleChange = (e) => {
        setValue(e.target.value);
        updateConfig(stack, e.target.value);
    };

    useEffect(() => {
        setValue(section.value);
    }, [section.value]);

    const isMistake = stack[stack.length - 1] === configMistake;

    return (
        <>
            <label>{section.title}</label>
            <TextInput error={isMistake} value={value} onChange={handleChange} />
        </>
    );
});

const RenderConfig = ({ section, stack, updateConfig, configMistake }) => {
    const renderConfig = useCallback((section, stack, stackKey, configMistake) => {
        const newStack = [...stack, stackKey];

        switch (section.type) {
            case 'tabs':
                return <TabsSection section={section} renderConfig={renderConfig} stack={newStack} configMistake={configMistake} />;
            case 'section':
                console.log('section', section);
                return <SectionAccordion section={section} renderConfig={renderConfig} stack={newStack} configMistake={configMistake} />;
            case 'body':
                return <BodySection section={section} renderConfig={renderConfig} stack={newStack} configMistake={configMistake} />;
            case 'select':
                return <SelectInput section={section} stack={newStack} updateConfig={updateConfig} configMistake={configMistake} />;
            case 'number':
                return <NumberInputField section={section} stack={newStack} updateConfig={updateConfig} configMistake={configMistake} />;
            case 'string':
                return <StringInputField section={section} stack={newStack} updateConfig={updateConfig} configMistake={configMistake} />;
            default:
                return null;
        }
    }, [updateConfig]);

    return renderConfig(section, stack, 'section', configMistake);
};

// @ts-ignore
export function Config({ config, updateConfig, configMistake }) {
    const renderedConfig = useMemo(() => (
        config ? <RenderConfig section={config.section} stack={[]} updateConfig={updateConfig} configMistake={configMistake} /> : null
    ), [config, updateConfig, configMistake]);

    return <div>{renderedConfig}</div>;
}
