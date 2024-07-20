import { viteBundler } from '@vuepress/bundler-vite';
import { defineUserConfig } from 'vuepress';
import { logoPath, repoBase, repoName, repoUrl } from './params';
import { hopeTheme } from 'vuepress-theme-hope';

const tutorialPath = '/md/';
const srcCodePath = tutorialPath + '详细分析/';

export default defineUserConfig({
    base: repoBase + '/',
    title: repoName,
    description: repoName,
    lang: 'zh-CN',
    theme: hopeTheme({
        sidebar: [
            { text: '首页', link: '/', },
            { text: '阅读须知', link: tutorialPath },
            { text: '基本概念', link: tutorialPath + '01基本概念', },
            { text: '使用线程', link: tutorialPath + '02使用线程', },
            { text: '共享数据', link: tutorialPath + '03共享数据', },
            { text: '同步操作', link: tutorialPath + '04同步操作', },
            { text: '内存模型与原子操作', link: tutorialPath + '05内存模型与原子操作', },
            { text: '协程', link: tutorialPath + '06协程', },
            {
                text: '详细分析', link: srcCodePath,
                collapsible: true,
                children: [
                    { text: 'std::thread 的构造-源码解析', link: srcCodePath + '01thread的构造与源码解析', },
                    { text: 'std::scoped_lock 的源码实现与解析', link: srcCodePath + '02scoped_lock源码解析', },
                    { text: 'std::async 与 std::future 源码解析', link: srcCodePath + '03async与future源码解析', },
                    { text: '线程池', link: srcCodePath + "04线程池", },
                ]
            },
        ],
        favicon: logoPath,
        logo: logoPath,
        navTitle: repoName,
        repo: repoUrl,
        editLinkPattern: repoUrl + 'blob/main/:path',
        contributors: false,
        darkmode: 'toggle',
        pageInfo: ['ReadingTime'],
        plugins: {
            mdEnhance: {
                gfm: true,
                hint: true,
                vPre: true,
                alert: true,
                tabs: true,
                codetabs: true,
                align: true,
                attrs: true,
                sup: true,
                sub: true,
                footnote: true,
                mark: true,
                figure: true,
                imgLazyload: true,
                imgMark: true,
                imgSize: true,
                obsidianImgSize: true,
                tasklist: true,
                include: true,
                katex: true,
                component: true,
                chart: true,
                echarts: true,
                flowchart: true,
                mermaid: true,
                plantuml: true,
            },
            searchPro: true
        }
    }),
    bundler: viteBundler({}),
})
